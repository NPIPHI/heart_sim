//
// Created by 16182 on 8/3/2021.
//

#ifndef HEART_SIM_COMPUTEPASS_H
#define HEART_SIM_COMPUTEPASS_H

#include<vulkan/vulkan.hpp>
#include "Buffer.h"

class ComputePass {
public:
    ComputePass(vk::PhysicalDevice physical_device, vk::Device device);
    void record(vk::CommandBuffer command_buffer, Buffer &src, Buffer &dst, Buffer &edges);
    vk::UniqueCommandBuffer create_command_buffer();
    vk::Queue queue();
private:
    vk::UniquePipeline create_compute_pipeline();
    vk::Device _device;
    vk::Queue _queue;
    vk::UniqueCommandPool _command_pool;
    vk::UniqueDescriptorPool _descriptor_pool;
    vk::UniqueDescriptorSetLayout _descriptor_set_layout;
    vk::UniqueDescriptorSet _descriptor_set;
    vk::UniqueShaderModule _shader_module;
    vk::UniquePipelineLayout _pipeline_layout;
    vk::UniquePipeline _pipeline;
};


#endif //HEART_SIM_COMPUTEPASS_H
