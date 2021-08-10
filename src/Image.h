//
// Created by 16182 on 7/19/2021.
//

#ifndef HEART_SIM_IMAGE_H
#define HEART_SIM_IMAGE_H
#include<vulkan/vulkan.hpp>
#include<vk_mem_alloc.h>

class Image {
public:
    Image() = default;
    Image(VmaAllocator allocator, vk::Extent2D extent, vk::Format format, vk::ImageUsageFlags image_usage, vk::ImageAspectFlags aspect_mask, VmaMemoryUsage memory_usage){
        VkImageCreateInfo image_create_info{
            .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = static_cast<VkFormat>(format),
            .extent = {extent.width, extent.height, 1},
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = static_cast<VkImageUsageFlags>(image_usage),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
        };
        VmaAllocationCreateInfo allocation_create_info{
            .usage = memory_usage
        };
        VkImage image;
        VmaAllocation allocation;
        vmaCreateImage(allocator, &image_create_info, &allocation_create_info, &image, &allocation, nullptr);
        _image = image;
        _allocation = allocation;
        _allocator = allocator;
        _extent = extent;
        _format = vk::Format(format);
        _aspect_mask = vk::ImageAspectFlags(aspect_mask);
    }
    Image(Image && src) noexcept : _image(src._image), _allocator(src._allocator), _allocation(src._allocation),
                                   _extent(src._extent), _format(src._format), _aspect_mask(src._aspect_mask){
        src._image = vk::Image{};
        src._allocator = {};
        src._allocation = {};
        src._extent = vk::Extent2D{};
        src._format = {};
        src._aspect_mask = {};
    }
    Image & operator=(Image && src) noexcept{
        _image = src._image;
        _allocator = src._allocator;
        _allocation = src._allocation;
        _extent = src._extent;
        _format = src._format;
        _aspect_mask = src._aspect_mask;
        src._image = vk::Image{};
        src._allocator = {};
        src._allocation = {};
        src._extent = vk::Extent2D{};
        src._format = {};
        src._aspect_mask = {};

        return *this;
    }
    Image(Image &) = delete;
    ~Image(){
        if(_image){
            assert(_image && _allocator && _allocation);
            vmaDestroyImage(_allocator, _image, _allocation);
        } else {
            assert(!_image && !_allocator && !_allocation);
        }
    }
    vk::UniqueImageView create_view(vk::Device device) {
        vk::ImageViewCreateInfo create_info(
                {},
                _image,
                vk::ImageViewType::e2D,
                _format,
                {},
                {
                    _aspect_mask,
                    0, 1, 0, 1
                }
                );
        return device.createImageViewUnique(create_info);
    }
private:
    vk::Image _image{};
    VmaAllocator _allocator{};
    VmaAllocation _allocation{};
    vk::Extent2D _extent{};
    vk::Format _format{};
    vk::ImageAspectFlags _aspect_mask{};
};

#endif //HEART_SIM_IMAGE_H
