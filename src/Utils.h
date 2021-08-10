//
// Created by 16182 on 7/19/2021.
//

#ifndef HEART_SIM_UTILS_H
#define HEART_SIM_UTILS_H

#include<optional>
#include<cstdint>
#define GLFW_INCLUDE_VULKAN
#include<GLFW/glfw3.h>
#include<vulkan/vulkan.hpp>
#include "VmaUniqueAllocator.h"
#include<span>

namespace Utils {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphics_family;
        std::optional<uint32_t> present_family;
        std::optional<uint32_t> compute_family;
        [[nodiscard]] bool complete() const;
    };
    QueueFamilyIndices find_queue_families(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface);

    void gpu_memcpy_host_local(VmaAllocator allocator, VmaAllocation dst, void * data, size_t size);

    vk::UniqueSurfaceKHR make_surface(GLFWwindow * window, vk::Instance instance);

    GLFWwindow * make_window(int width, int height);

    vk::UniqueInstance make_instance(const char *const *validation_layers, size_t validation_layers_size);

    vk::PhysicalDevice pick_physical_device(const std::vector<vk::PhysicalDevice> &devices, vk::SurfaceKHR display_surface);

    vk::UniqueDevice make_device(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, const char * const * required_extensions, size_t extensions_size);

    UniqueAllocator make_allocator(vk::Instance instance, vk::PhysicalDevice physical_device, vk::Device device);
};


#endif //HEART_SIM_UTILS_H
