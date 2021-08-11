//
// Created by 16182 on 6/21/2021.
//

#ifndef HEART_SIM_VERTEX_H
#define HEART_SIM_VERTEX_H
#include<vulkan/vulkan.hpp>
#define GLM_FORCE_RADIANS
#include<GLM/vec2.hpp>
#include<GLM/vec3.hpp>
#include<GLM/mat4x4.hpp>
#include<array>

struct UniformBufferObject {
    glm::mat4 ubo;
};

struct SpecilizationData {
    uint32_t BUFFER_ELEMENT_COUNT;
};

struct Vertex {
    alignas(16) glm::vec3 pos;

    static vk::VertexInputBindingDescription binding_description() {
        VkVertexInputBindingDescription binding_description{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
        };
        return binding_description;
    }

    static std::array<vk::VertexInputAttributeDescription, 1> attribute_descriptions(){
        std::array<vk::VertexInputAttributeDescription, 1> attribute_descriptions{
            {
                vk::VertexInputAttributeDescription(
                    0,
                    0,
                    vk::Format::eR32G32B32A32Sfloat,
                    offsetof(Vertex, pos)
                ),
            }
        };
        return attribute_descriptions;
    }
};

#endif //HEART_SIM_VERTEX_H
