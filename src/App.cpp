//
// Created by 16182 on 7/14/2021.
//

#include "App.h"
#include<random>
#include<iostream>
#include<stl_reader.h>
#include<algorithm>
#define GLM_FORCE_RADIANS
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include "NodeGraph.h"
#include "FillPoints.h"
#include "NodeState.h"

App::App(int width, int height) :
        _width(width),
        _height(height),
        _window(Utils::make_window(width, height)),
        _instance(Utils::make_instance(_validation_layers.data(), _validation_layers.size())),
        _surface(Utils::make_surface(_window, *_instance)),
        _physical_device(Utils::pick_physical_device(_instance->enumeratePhysicalDevices(), *_surface)),
        _device(Utils::make_device(_physical_device, *_surface, _logical_device_extensions.data(), _logical_device_extensions.size())),
        _allocator(Utils::make_allocator(*_instance, _physical_device, *_device)),
        _present(_physical_device, *_device, *_surface, _window, 2),
        _graphics(*_allocator, _physical_device, *_device, *_surface, _present),
        _compute(_physical_device, *_device)
{
    double cursor_x_pos, cursor_y_pos;
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwGetCursorPos(_window, &cursor_x_pos, &cursor_y_pos);
    _view_state = {cursor_x_pos, cursor_y_pos};
    glfwSetWindowUserPointer(_window, this);

    glfwSetCursorPosCallback(_window, App::cursor_pos_callback);

    stl_reader::StlMesh<float, uint32_t> mesh("heart.stl");

    for(size_t i = 0; i < mesh.num_vrts(); i++){
        float x = mesh.raw_coords()[i*3];
        float y = mesh.raw_coords()[i*3+1];
        float z = mesh.raw_coords()[i*3+2];
        vertices.push_back({{x,y,z}});
    }

    for(size_t i = 0; i < mesh.num_tris(); i++){
        indices.push_back(mesh.raw_tris()[i*3]);
        indices.push_back(mesh.raw_tris()[i*3+1]);
        indices.push_back(mesh.raw_tris()[i*3+2]);
    }

    auto points = FillPoints::random_fill(mesh);
    for(auto p : points){
        vertices.push_back({p});
    }
    std::cout << vertices.size() << std::endl;

    std::vector<NodeState> node_states(vertices.size(), NodeState{});

    auto node_graph = NodeGraph{vertices};

    _compute_command_buffer = _compute.create_command_buffer();

    vertex_buffer = {*_allocator, vertices.size() * sizeof(vertices[0]), vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer, VMA_MEMORY_USAGE_GPU_ONLY};
    index_buffer = {*_allocator, indices.size() * sizeof(indices[0]), vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY};
    _graph_buffer = {*_allocator, node_graph.edges.size() * sizeof(node_graph.edges[0]), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY};
    node_state_buffer1 = {*_allocator, node_states.size() * sizeof(node_states[0]), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY};
    node_state_buffer2 = {*_allocator, node_states.size() * sizeof(node_states[0]), vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst, VMA_MEMORY_USAGE_GPU_ONLY};

    auto vertex_transfer_src = Buffer::from_transfer_data(*_allocator, vertices.data(), vertices.size() * sizeof(vertices[0]));
    auto graph_transfer_src = Buffer::from_transfer_data(*_allocator, node_graph.edges.data(), node_graph.edges.size() * sizeof(node_graph.edges[0]));
    auto index_transfer_src = Buffer::from_transfer_data(*_allocator, indices.data(), indices.size() * sizeof(indices[0]));
    auto node_state_transfer_src = Buffer::from_transfer_data(*_allocator, node_states.data(), node_states.size() * sizeof(node_states[0]));

    auto copy_command_buffer = _graphics.create_command_buffer();
    copy_command_buffer->begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit});
    copy_command_buffer->copyBuffer(vertex_transfer_src.vkBuffer(), vertex_buffer.vkBuffer(), vk::BufferCopy{0, 0, vertex_buffer.size()});
    copy_command_buffer->copyBuffer(graph_transfer_src.vkBuffer(), _graph_buffer.vkBuffer(), vk::BufferCopy{0, 0, _graph_buffer.size()});
    copy_command_buffer->copyBuffer(index_transfer_src.vkBuffer(), index_buffer.vkBuffer(), vk::BufferCopy{0, 0, index_buffer.size()});
    copy_command_buffer->copyBuffer(node_state_transfer_src.vkBuffer(), node_state_buffer1.vkBuffer(), vk::BufferCopy{0, 0, node_state_buffer1.size()});
    copy_command_buffer->copyBuffer(node_state_transfer_src.vkBuffer(), node_state_buffer2.vkBuffer(), vk::BufferCopy{0, 0, node_state_buffer2.size()});
    copy_command_buffer->end();
    vk::SubmitInfo submit_info;
    submit_info.pCommandBuffers = &copy_command_buffer.get();
    submit_info.commandBufferCount = 1;
    auto copy_done_fence = _device->createFenceUnique({});
    _graphics.queue().submit(submit_info, *copy_done_fence);
    (void)_device->waitForFences(*copy_done_fence, VK_TRUE, UINT64_MAX);
}

void App::draw_frame(Buffer &node_state_buffer) {
    static size_t frame_num = 0;
    frame_num++;
    uint32_t image_index = _present.get_next_image_index();
    vk::UniqueCommandBuffer command_buffer = _graphics.create_command_buffer();
    vk::CommandBufferBeginInfo begin_info{};

    auto descriptor_set = _graphics.create_descriptor_set();
    _graphics.write_descriptor(*descriptor_set, node_state_buffer);
    command_buffer->begin(begin_info);
    _graphics.record_begin_render(*command_buffer, _graphics.get_framebuffer(image_index));
    auto mvp = _view_state.mvp(_width, _height);
    _graphics.record_push_constants(*command_buffer, &mvp, sizeof(mvp));
    _graphics.record_bind_descriptors(*command_buffer, *descriptor_set);
    _graphics.record_draw_indexed(*command_buffer, vertex_buffer, index_buffer, indices.size());
    _graphics.record_end_render(*command_buffer);
    command_buffer->end();

    vk::Semaphore wait_semaphores[] = {
            _present.image_available_semaphore(),
    };
    vk::Semaphore signal_semaphores[] = {
            _present.render_finished_semaphore()
    };
    vk::PipelineStageFlags wait_stages = vk::PipelineStageFlagBits::eColorAttachmentOutput;

    vk::SubmitInfo submit_info(
            std::size(wait_semaphores),
            wait_semaphores,
            &wait_stages,
            1,
            &*command_buffer,
            std::size(signal_semaphores),
            signal_semaphores
            );
    _device->resetFences(_present.in_flight_fence());
    _graphics.queue().submit(submit_info, _present.in_flight_fence());
    _present.present_image(image_index);
    _device->waitIdle();
}

void App::run() {
    while(true){
        glfwPollEvents();
        if(glfwWindowShouldClose(_window)) break;
        if(glfwGetKey(_window, GLFW_KEY_ESCAPE)) break;

        bool forwards = glfwGetKey(_window, GLFW_KEY_W);
        bool backwards = glfwGetKey(_window, GLFW_KEY_S);
        bool left = glfwGetKey(_window, GLFW_KEY_A);
        bool right = glfwGetKey(_window, GLFW_KEY_D);
        bool up = glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT);
        bool down = glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL);
        _view_state.update_key_press(forwards, backwards, left, right, up, down, 1.f / 120.f);

        if(frame_count % 2 == 0){
            update_nodes(node_state_buffer1, node_state_buffer2);
            draw_frame(node_state_buffer1);
        } else {
            update_nodes(node_state_buffer2, node_state_buffer1);
            draw_frame(node_state_buffer2);
        }

        frame_count++;
    }
}

void App::cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
    App* app = (App*)glfwGetWindowUserPointer(window);
    app->update_cursor_pos(xpos, ypos);
}

void App::update_cursor_pos(double xpos, double ypos) {
    _view_state.update_cursor_pos(xpos, ypos);
}

void App::update_nodes(Buffer &src, Buffer &dst) {
    _compute_command_buffer->reset({});
    auto & command_buffer = _compute_command_buffer;
    vk::CommandBufferBeginInfo begin_info{};
    command_buffer->begin(begin_info);
    _compute.record(*command_buffer, src, dst, _graph_buffer);
    command_buffer->dispatch(vertices.size()/32+1, 1, 1);
    command_buffer->end();
    vk::SubmitInfo submit_info(
            0, nullptr,
            nullptr,
            1, &*command_buffer,
            0, nullptr
            );

    _compute.queue().submit(submit_info);
}
