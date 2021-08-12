//
// Created by 16182 on 7/14/2021.
//

#ifndef HEART_SIM_APP_H
#define HEART_SIM_APP_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include "VmaUniqueAllocator.h"
#include "Buffer.h"
#include "Image.h"
#include "Vertex.h"
#include "GraphicsPass.h"
#include "PresentPass.h"
#include "ComputePass.h"
#include "ViewState.h"
#include "NodeGraph.h"

class App {
public:
    App(int width, int height);
    App(App &) = delete;
    App(App &&) = delete;

    void run();
private:
    static void cursor_pos_callback(GLFWwindow * window, double xpos, double ypos);
    void update_cursor_pos(double xpos, double ypos);
    void draw_frame(Buffer &node_state_buffer);
    void update_nodes(Buffer &src, Buffer &dst);
    int _width, _height;
    GLFWwindow * _window;
    vk::UniqueInstance _instance;
    vk::UniqueSurfaceKHR _surface;
    vk::PhysicalDevice _physical_device;
    vk::UniqueDevice _device;
    UniqueAllocator _allocator;
    PresentPass _present;
    GraphicsPass _graphics;
    ComputePass _compute;
    ViewState _view_state;
    Buffer _graph_buffer;
    vk::UniqueCommandBuffer _compute_command_buffer;
    size_t frame_count = 0;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    size_t node_count;
    Buffer vertex_buffer, index_buffer, node_state_buffer1, node_state_buffer2;

    constexpr static std::array<const char *, 1> _logical_device_extensions = {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
#ifdef NDEBUG
    constexpr static std::array<const char *, 0> _validation_layers = {

    };
#else
    constexpr static std::array<const char *, 2> _validation_layers = {{
            "VK_LAYER_KHRONOS_validation",
            "VK_LAYER_KHRONOS_synchronization2",
    }};
#endif
};


#endif //HEART_SIM_APP_H
