//
// Created by 16182 on 7/29/2021.
//

#ifndef HEART_SIM_PRESENTPASS_H
#define HEART_SIM_PRESENTPASS_H

#include<vulkan/vulkan.hpp>
#include<GLFW/glfw3.h>
#include<vector>
#include<span>
class PresentPass {
public:
    PresentPass() = default;
    PresentPass(vk::PhysicalDevice physical_device, vk::Device device, vk::SurfaceKHR surface,
                GLFWwindow *window, size_t image_count);
    PresentPass(PresentPass &) = delete;
    PresentPass(PresentPass &&) = delete;

    uint32_t get_next_image_index();

    vk::ImageView acquire_image(uint32_t index);

    std::vector<vk::ImageView> image_views() const;

    vk::Format image_format() const {
        return _image_format;
    }

    vk::Extent2D extent() const {
        return _extent;
    }

    void present_image(uint32_t index);

    vk::Semaphore image_available_semaphore();

    vk::Semaphore render_finished_semaphore();

    vk::Fence in_flight_fence();

private:
    static std::vector<vk::UniqueImageView> make_present_image_views(vk::Device device, std::span<vk::Image> images, vk::Format format);
    vk::Device _device;
    vk::UniqueSwapchainKHR _swap_chain;
    vk::Format _image_format;
    vk::Extent2D _extent;
    vk::Queue _queue;
    size_t _current_frame{};
    vk::UniqueFence _in_flight_fence;
    std::vector<vk::Fence> _images_in_flight_fences;
    vk::UniqueSemaphore _image_available, _render_finished;
    std::vector<vk::Image> _images;
    std::vector<vk::UniqueImageView> _image_views;
    std::vector<vk::UniqueFramebuffer> _frame_buffers;
};


#endif //HEART_SIM_PRESENTPASS_H
