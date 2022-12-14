#pragma once

#include <toy_renderer/api/api_texture.h>

#include <toy_renderer/handle.h>

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace ToyRenderer {

class VulkanResourceManager;

struct Device_t;

struct VulkanTexture : public ApiTexture {
    explicit VulkanTexture(VkImage _image,
                           VmaAllocation _allocation,
                           VulkanResourceManager *_vulkanResourceManager,
                           const Handle<Device_t> &_deviceHandle);

    VkImage image{ VK_NULL_HANDLE };
    VmaAllocation allocation{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace ToyRenderer
