//
// Created by 16182 on 7/29/2021.
//

#include "PresentPass.h"
#include "Utils.h"
#include<algorithm>
vk::Extent2D get_extent(vk::SurfaceCapabilitiesKHR &capabilites, GLFWwindow *window) {
    if(capabilites.currentExtent.width != std::numeric_limits<uint32_t>::max()){
        return capabilites.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    auto extent = vk::Extent2D(
            std::clamp((uint32_t)width, capabilites.minImageExtent.width, capabilites.
            maxImageExtent.width),
            std::clamp((uint32_t)height, capabilites.minImageExtent.height, capabilites.
            maxImageExtent.height)
            );
    return extent;
}

vk::Format pick_present_format(vk::PhysicalDevice physical_device, vk::SurfaceKHR surface){
    auto formats = physical_device.getSurfaceFormatsKHR(surface);
    assert(!formats.empty());
    for(auto format : formats){
        if(format.format == vk::Format::eR8G8B8A8Srgb){
            return format.format;
        }
    }
    return formats[0].format;
}

PresentPass::PresentPass(vk::PhysicalDevice physical_device, vk::Device device, vk::SurfaceKHR surface,
                         GLFWwindow *window, size_t image_count) {
    auto queue_indices = Utils::find_queue_families(physical_device, surface);
    _queue = device.getQueue(queue_indices.present_family.value(), 0);
    _device = device;
    _image_format = pick_present_format(physical_device, surface);
    _render_finished = device.createSemaphoreUnique({});
    auto present_mode = vk::PresentModeKHR::eMailbox;
    auto capabilites = physical_device.getSurfaceCapabilitiesKHR(surface);
    _extent = get_extent(capabilites, window);

    if(image_count < capabilites.minImageCount || image_count > capabilites.maxImageCount){
        throw std::runtime_error("image count out of range");
    }
    vk::SwapchainCreateInfoKHR create_info(
            {},
            surface,
            image_count,
            _image_format,
            vk::ColorSpaceKHR::eSrgbNonlinear,
            _extent,
            1,
            vk::ImageUsageFlagBits::eColorAttachment,
            vk::SharingMode::eExclusive,
            0,
            nullptr,
            vk::SurfaceTransformFlagBitsKHR::eIdentity,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            present_mode,
            VK_TRUE
            );

    _swap_chain = device.createSwapchainKHRUnique(create_info);
    _images = device.getSwapchainImagesKHR(*_swap_chain);
    _image_views = make_present_image_views(device, _images, _image_format);
    _images_in_flight_fences.resize(_images.size(), nullptr);
    _in_flight_fence = device.createFenceUnique({vk::FenceCreateFlagBits::eSignaled});
    _image_available = device.createSemaphoreUnique({});
}

std::vector<vk::UniqueImageView>
PresentPass::make_present_image_views(vk::Device device, std::span<vk::Image> images, vk::Format format) {
    std::vector<vk::UniqueImageView> image_views;
    for(auto image : images){
        vk::ImageViewCreateInfo create_info(
                {},
                image,
                vk::ImageViewType::e2D,
                format,
                {},
                {
                    vk::ImageAspectFlagBits::eColor,
                    0,
                    1,
                    0,
                    1
                }
                );
        image_views.push_back(device.createImageViewUnique(create_info));
    }
    return image_views;
}

uint32_t PresentPass::get_next_image_index() {
    (void)_device.waitForFences(*_in_flight_fence, VK_TRUE, UINT64_MAX);
    uint32_t index = _device.acquireNextImageKHR(*_swap_chain, UINT64_MAX, *_image_available).value;
    return index;
}

vk::ImageView PresentPass::acquire_image(uint32_t index) {
    if(_images_in_flight_fences[index]){
        (void)_device.waitForFences(_images_in_flight_fences[index], VK_TRUE, UINT64_MAX);
    }
    _images_in_flight_fences[index] = *_in_flight_fence;
    return *_image_views[index];
}

std::vector<vk::ImageView> PresentPass::image_views() const{
    std::vector<vk::ImageView> image_views(_image_views.size());
    for(size_t i = 0; i < _image_views.size(); i++){
        image_views[i] = *_image_views[i];
    }
    return image_views;
}

vk::Semaphore PresentPass::image_available_semaphore() {
    return *_image_available;
}

vk::Semaphore PresentPass::render_finished_semaphore() {
    return *_render_finished;
}

vk::Fence PresentPass::in_flight_fence() {
    return *_in_flight_fence;
}

void PresentPass::present_image(uint32_t index) {
    vk::PresentInfoKHR present_info(
            1,
            &*_render_finished,
            1,
            &*_swap_chain,
            &index
            );

    (void)_queue.presentKHR(present_info);
}
