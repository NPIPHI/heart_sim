//
// Created by 16182 on 8/3/2021.
//

#include "ComputePass.h"
#include "LoadAssets.h"
#include "Utils.h"
#include "Buffer.h"

ComputePass::ComputePass(vk::PhysicalDevice physical_device, vk::Device device)
{
    auto queue_indices = Utils::find_queue_families(physical_device, {});
    _device = device;
    _queue = device.getQueue(queue_indices.compute_family.value(), 0);
    _command_pool = device.createCommandPoolUnique({vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient, queue_indices.compute_family.value()});
    auto shader_code = LoadAssets::read_file("comp.comp.spv");
    _shader_module = device.createShaderModuleUnique({{}, shader_code.size(), (const uint32_t*)shader_code.data()});
    vk::DescriptorPoolSize pool_sizes[]{
            {vk::DescriptorType::eStorageBuffer, 3}
    };
    _descriptor_pool = device.createDescriptorPoolUnique({vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, std::size(pool_sizes), pool_sizes});
//    device.createDescriptorSetLayout()

    std::array<vk::DescriptorSetLayoutBinding, 3> set_layout_bindings{{
        {
            0,
            vk::DescriptorType::eStorageBuffer,
            1,
            vk::ShaderStageFlagBits::eCompute,
            nullptr
        },
        {
            1,
            vk::DescriptorType::eStorageBuffer,
            1,
            vk::ShaderStageFlagBits::eCompute,
            nullptr
        },
        {
            2,
            vk::DescriptorType::eStorageBuffer,
            1,
            vk::ShaderStageFlagBits::eCompute,
            nullptr
        }
    }};

    _descriptor_set_layout = device.createDescriptorSetLayoutUnique({{}, set_layout_bindings});
    _descriptor_set = std::move(device.allocateDescriptorSetsUnique({*_descriptor_pool, 1, &*_descriptor_set_layout})[0]);
    _pipeline_layout = device.createPipelineLayoutUnique({{}, 1, &*_descriptor_set_layout});
    _pipeline = create_compute_pipeline();
}

vk::UniquePipeline ComputePass::create_compute_pipeline() {
    vk::PipelineShaderStageCreateInfo shader_stage_create_info(
            {},
            vk::ShaderStageFlagBits::eCompute,
            *_shader_module,
            "main"
            );
    vk::ComputePipelineCreateInfo compute_pipeline_create_info({}, shader_stage_create_info, *_pipeline_layout);
    return _device.createComputePipelineUnique({}, compute_pipeline_create_info).value;
}

void ComputePass::record(vk::CommandBuffer command_buffer, Buffer &src, Buffer &dst, Buffer &edges) {
    assert(src.size() == dst.size());
    auto src_info = src.buffer_info();
    auto dst_info = dst.buffer_info();
    auto graph_info = edges.buffer_info();
    vk::WriteDescriptorSet writes[]{
            {
                *_descriptor_set,
                0,
                0,
                1,
                vk::DescriptorType::eStorageBuffer,
                nullptr,
                &src_info
            },
            {
                *_descriptor_set,
                1,
                0,
                1,
                vk::DescriptorType::eStorageBuffer,
                nullptr,
                &dst_info
            },
            {
                *_descriptor_set,
                2,
                0,
                1,
                vk::DescriptorType::eStorageBuffer,
                nullptr,
                &graph_info
            }
    };
    _device.updateDescriptorSets(std::size(writes), writes, 0, nullptr);

    command_buffer.bindPipeline(vk::PipelineBindPoint::eCompute, *_pipeline);
    command_buffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *_pipeline_layout, 0, *_descriptor_set, {});
}

vk::UniqueCommandBuffer ComputePass::create_command_buffer() {
    return std::move(_device.allocateCommandBuffersUnique({*_command_pool, vk::CommandBufferLevel::ePrimary, 1})[0]);
}

vk::Queue ComputePass::queue() {
    return _queue;
}
