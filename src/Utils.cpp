//
// Created by 16182 on 7/19/2021.
//

#include <span>
#include "Utils.h"

bool Utils::QueueFamilyIndices::complete() const {
    return graphics_family.has_value() && present_family.has_value() && compute_family.has_value();
}

Utils::QueueFamilyIndices Utils::find_queue_families(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface) {
    QueueFamilyIndices indices{};
    std::vector<vk::QueueFamilyProperties> queue_families = physical_device.getQueueFamilyProperties();
    for(size_t i = 0; i < queue_families.size(); i++){
        if(queue_families[i].queueFlags & vk::QueueFlagBits::eGraphics){
            if(!indices.graphics_family.has_value())
                indices.graphics_family = i;
        }
        if(queue_families[i].queueFlags & vk::QueueFlagBits::eCompute){
            if(!indices.compute_family.has_value())
                indices.compute_family = i;
        }
        if(surface){
            auto present_support = physical_device.getSurfaceSupportKHR(i, surface);
            if(present_support) {
                if(!indices.present_family.has_value())
                    indices.present_family = i;
            }
        }
    }
    return indices;
}

void Utils::gpu_memcpy_host_local(VmaAllocator allocator, VmaAllocation dst, void *data, size_t size) {
    if(size == 0) return;
    //vmaMapMemory fails on cpu mapped memory where the memory region is 0 bytes
    void * write_ptr;
    auto result = vmaMapMemory(allocator, dst, &write_ptr);
    if(result != VK_SUCCESS){
        throw std::runtime_error("failed to map memory");
    }
    std::memcpy(write_ptr, data, size);
    vmaUnmapMemory(allocator, dst);
}

vk::UniqueSurfaceKHR Utils::make_surface(GLFWwindow *window, vk::Instance instance) {
    VkSurfaceKHR surface;
    auto result = glfwCreateWindowSurface(instance, window, nullptr, &surface);
    if(result != VK_SUCCESS){
        throw std::runtime_error("failed to create window surface");
    }
    return vk::UniqueSurfaceKHR(surface, instance);
}

GLFWwindow *Utils::make_window(int width, int height) {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window = glfwCreateWindow(width, height, "heart sim", nullptr, nullptr);
    if(window){
        return window;
    } else {
        throw std::runtime_error("failed to create window");
    }
}

vk::UniqueInstance Utils::make_instance(const char *const *validation_layers, size_t validation_layers_size) {
    uint32_t extension_count;
    auto extensions = glfwGetRequiredInstanceExtensions(&extension_count);
    vk::ApplicationInfo application_info(
            "heart sim",
            VK_MAKE_VERSION(1, 0, 0),
            "none",
            VK_MAKE_VERSION(1, 0, 0),
            VK_API_VERSION_1_2
    );
    vk::InstanceCreateInfo instance_create_info(
            {},
            &application_info,
            validation_layers_size,
            validation_layers,
            extension_count,
            extensions
    );
    return vk::createInstanceUnique(instance_create_info);
}

vk::PhysicalDevice Utils::pick_physical_device(const std::vector<vk::PhysicalDevice> &devices, vk::SurfaceKHR display_surface) {
    for(auto & d : devices){
        if ((d.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)){
            return d;
        }
    }
    if(!devices.empty()){
        return devices.front();
    }
    throw std::runtime_error("no physical devices");
}

vk::UniqueDevice Utils::make_device(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface, const char *const *required_extensions,
                   size_t extensions_size) {
    auto indices = find_queue_families(physical_device, surface);
    std::vector<uint32_t> unique_queue_families = {indices.graphics_family.value(), indices.present_family.value()};
    unique_queue_families.erase(std::unique(unique_queue_families.begin(), unique_queue_families.end()), unique_queue_families.end());
    std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
    float queue_priority = 1;

    for(auto queue_family : unique_queue_families){
        queue_create_infos.push_back({{},queue_family,1,&queue_priority});
    }

    vk::PhysicalDeviceFeatures device_features{};

    vk::DeviceCreateInfo device_create_info(
            {},
            queue_create_infos.size(),
            queue_create_infos.data(),
            {},
            {},
            extensions_size,
            required_extensions,
            &device_features
            );

    return physical_device.createDeviceUnique(device_create_info);
}

UniqueAllocator Utils::make_allocator(vk::Instance instance, vk::PhysicalDevice physical_device, vk::Device device) {
    VmaAllocatorCreateInfo create_info{
        .physicalDevice = physical_device,
        .device = device,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_2,
    };

    VmaAllocator allocator;
    vmaCreateAllocator(&create_info, &allocator);
    return UniqueAllocator(allocator);
}
