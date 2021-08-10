//
// Created by 16182 on 7/19/2021.
//

#include "GraphicsPass.h"
#include "LoadAssets.h"
#include "Utils.h"
#include "Vertex.h"
#include<array>

GraphicsPass::GraphicsPass(VmaAllocator allocator, vk::PhysicalDevice physical_device, vk::Device device,
                           vk::SurfaceKHR surface, PresentPass const &present_pass) {
    _device = device;
    _extent = present_pass.extent();
    auto vert_code = LoadAssets::read_file("tri.vert.spv");
    auto frag_code = LoadAssets::read_file("tri.frag.spv");
    _vert_shader = device.createShaderModuleUnique({{}, vert_code.size(), (const uint32_t*)vert_code.data()});
    _frag_shader = device.createShaderModuleUnique({{}, frag_code.size(), (const uint32_t*)frag_code.data()});

    auto queue_indices = Utils::find_queue_families(physical_device, surface);
    _queue = device.getQueue(queue_indices.graphics_family.value(), 0);
    _command_pool = device.createCommandPoolUnique({{}, queue_indices.graphics_family.value()});

    std::array<vk::DescriptorSetLayoutBinding, 0> set_layout_bindings{};
//    std::array<vk::DescriptorSetLayoutBinding, 2> set_layout_bindings{{
//        {
//            0,
//            vk::DescriptorType::eUniformBuffer,
//            1,
//            vk::ShaderStageFlagBits::eVertex,
//            nullptr
//        },
//        {
//            1,
//            vk::DescriptorType::eCombinedImageSampler,
//            1,
//            vk::ShaderStageFlagBits::eFragment,
//            nullptr
//        }
//    }};
    _descriptor_set_layout = device.createDescriptorSetLayoutUnique({{}, set_layout_bindings});

    std::array<vk::DescriptorPoolSize, 1> pool_sizes{{
     {vk::DescriptorType::eUniformBuffer, 1}
    }};
    _depth_image = Image(allocator, _extent, vk::Format::eD32Sfloat, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageAspectFlagBits::eDepth, VMA_MEMORY_USAGE_GPU_ONLY);
    _depth_image_view = _depth_image.create_view(_device);
    _descriptor_pool = device.createDescriptorPoolUnique(vk::DescriptorPoolCreateInfo{vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, pool_sizes});
    _descriptor_set = std::move(device.allocateDescriptorSetsUnique({*_descriptor_pool, *_descriptor_set_layout})[0]);


//    vk::DescriptorBufferInfo buffer_info(
//                _vertex_buffer.vkBuffer()
//            )
//    vk::WriteDescriptorSet desecriptor_writes[]{
//        vk::WriteDescriptorSet{
//            *_descriptor_set,
//            0,
//            0,
//            1,
//            vk::DescriptorType::eUniformBuffer,
//            nullptr,
//        }
//    };

    vk::PushConstantRange push_constant_range(
            vk::ShaderStageFlagBits::eVertex,
            0,
            sizeof(glm::mat4)
            );
    vk::PipelineLayoutCreateInfo pipeline_layout_create_info(
            {},
            1,
            &*_descriptor_set_layout,
            1,
            &push_constant_range
            );
    _pipeline_layout = device.createPipelineLayoutUnique(pipeline_layout_create_info);
    _render_pass = make_render_pass(device, present_pass.image_format(), vk::Format::eD32Sfloat);
    _pipeline = make_pipeline(device, _extent);
    auto image_views = present_pass.image_views();
    _frame_buffers = make_framebuffers(image_views);

}

vk::UniquePipeline GraphicsPass::make_pipeline(vk::Device device, vk::Extent2D extent) {
    vk::PipelineShaderStageCreateInfo shader_stage_infos[]{
            {
                {},
                vk::ShaderStageFlagBits::eVertex,
                *_vert_shader,
                "main"
            },
            {
                {},
                vk::ShaderStageFlagBits::eFragment,
                *_frag_shader,
                "main"
            }
    };

    auto binding_description = Vertex::binding_description();
    auto attribute_descriptions = Vertex::attribute_descriptions();
    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info(
            {},
            binding_description,
            attribute_descriptions
            );

    vk::PipelineInputAssemblyStateCreateInfo assembly_state_create_info(
            {},
            vk::PrimitiveTopology::eTriangleList
            );

    vk::Viewport viewport(
            0,
            0,
            (float)extent.width,
            (float)extent.height,
            0,
            1
            );

    vk::Rect2D scissor(
            {0, 0},
            extent
            );

    vk::PipelineViewportStateCreateInfo viewport_state_create_info({}, viewport, scissor);

    vk::PipelineRasterizationStateCreateInfo rasterization_state_create_info{};
    rasterization_state_create_info.cullMode = vk::CullModeFlagBits::eBack;
    rasterization_state_create_info.frontFace = vk::FrontFace::eClockwise;
    rasterization_state_create_info.lineWidth = 1.f;

    vk::PipelineMultisampleStateCreateInfo multisample_state_create_info(
            {},
            vk::SampleCountFlagBits::e1,
            VK_FALSE,
            1.f
            );

    vk::PipelineDepthStencilStateCreateInfo depth_stencil_state_create_info(
            {},
            VK_TRUE,
            VK_TRUE,
            vk::CompareOp::eLess
            );


    vk::PipelineColorBlendAttachmentState color_blend_attachment_state(
            {},
            {},
            {},
            {},
            {},
            {},
            {},
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
    );

    vk::PipelineColorBlendStateCreateInfo color_blend_state_create_info(
            {},
            {},
            vk::LogicOp::eCopy,
            color_blend_attachment_state
            );

    vk::GraphicsPipelineCreateInfo create_info(
            {},
            std::size(shader_stage_infos),
            shader_stage_infos,
            &vertex_input_state_create_info,
            &assembly_state_create_info,
            {},
            &viewport_state_create_info,
            &rasterization_state_create_info,
            &multisample_state_create_info,
            &depth_stencil_state_create_info,
            &color_blend_state_create_info,
            {},
            *_pipeline_layout,
            *_render_pass,
            0,
            {},
            -1);

    return device.createGraphicsPipelineUnique({}, create_info).value;
}

vk::UniqueRenderPass GraphicsPass::make_render_pass(vk::Device device, vk::Format image_format, vk::Format depth_format) {
    vk::AttachmentDescription color_attachment(
            {},
            image_format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eStore,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR
    );

    vk::AttachmentDescription depth_attachment{
            {},
            depth_format,
            vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear,
            vk::AttachmentStoreOp::eDontCare,
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::eDepthStencilAttachmentOptimal
    };

    vk::AttachmentReference color_attachment_ref(
            0,
            vk::ImageLayout::eColorAttachmentOptimal
    );

    vk::AttachmentReference depth_attachment_ref(
            1,
            vk::ImageLayout::eDepthStencilAttachmentOptimal
    );

    vk::SubpassDescription subpass;
    subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    vk::SubpassDependency dependency(
            VK_SUBPASS_EXTERNAL,
            0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
            vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
            vk::AccessFlagBits::eNoneKHR,
            vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite,
            {}
            );
//
//    VkSubpassDependency dependency{
//            .srcSubpass = VK_SUBPASS_EXTERNAL,
//            .dstSubpass = 0,
//            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//            .srcAccessMask = 0,
//            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
//    };

    vk::AttachmentDescription attachments[] = {color_attachment, depth_attachment};

    vk::RenderPassCreateInfo render_pass_create_info(
            {},
            std::size(attachments), attachments,
            1, &subpass,
            1, &dependency
            );
    return device.createRenderPassUnique(render_pass_create_info);
}

vk::UniqueCommandBuffer GraphicsPass::create_command_buffer() {
    return std::move(_device.allocateCommandBuffersUnique({*_command_pool, vk::CommandBufferLevel::ePrimary, 1})[0]);
}

std::vector<vk::UniqueFramebuffer> GraphicsPass::make_framebuffers(std::span<vk::ImageView> image_views) {
    std::vector<vk::UniqueFramebuffer> frame_buffers;

    for(auto image : image_views){
        vk::ImageView attachments[]{
            image,
            *_depth_image_view
        };

        vk::FramebufferCreateInfo framebuffer_create_info(
                {},
                *_render_pass,
                std::size(attachments),
                attachments,
                _extent.width,
                _extent.height,
                1
                );

        frame_buffers.push_back(_device.createFramebufferUnique(framebuffer_create_info));
    }

    return frame_buffers;
}

vk::Framebuffer GraphicsPass::get_framebuffer(uint32_t index) {
    return *_frame_buffers[index];
}

void GraphicsPass::record_draw(vk::CommandBuffer command_buffer, const Buffer &vertex_buffer, size_t vertex_count) {
    vk::DeviceSize offsets[] = {0};
    vk::Buffer vertex_buffers[] = {vertex_buffer.vkBuffer()};
    command_buffer.bindVertexBuffers(0, 1, vertex_buffers, offsets);
    command_buffer.draw(vertex_count, 1, 0, 0);
}

void GraphicsPass::record_draw_indexed(vk::CommandBuffer command_buffer, const Buffer &vertex_buffer,
                                       const Buffer &index_buffer, size_t index_count) {
    vk::Buffer vertex_buffers[] = {vertex_buffer.vkBuffer()};
    vk::DeviceSize offsets[] = {0};
    command_buffer.bindVertexBuffers(0, 1, vertex_buffers, offsets);
    command_buffer.bindIndexBuffer(index_buffer.vkBuffer(), 0, vk::IndexType::eUint32);
    command_buffer.drawIndexed(index_count, 1, 0, 0, 0);
}

void GraphicsPass::record_begin_render(vk::CommandBuffer command_buffer, vk::Framebuffer frame_buffer) {
    vk::ClearValue clear_values[2]{};
    clear_values[0].color.float32 = std::array<float,4>{0, 0, 0, 1};
    clear_values[1].depthStencil.depth = 1.f;
    clear_values[1].depthStencil.stencil = 0.f;

    vk::RenderPassBeginInfo render_pass_begin_info(
            *_render_pass,
            frame_buffer,
            {
                {0, 0},
                _extent,
                },
                std::size(clear_values),
                clear_values
                );

    command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
    command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *_pipeline);
}

void GraphicsPass::record_end_render(vk::CommandBuffer command_buffer) {
    command_buffer.endRenderPass();
}

void GraphicsPass::record_push_constants(vk::CommandBuffer command_buffer, void *data, size_t size) {
    command_buffer.pushConstants(*_pipeline_layout, vk::ShaderStageFlagBits::eVertex, 0, size, data);
}

