#pragma once

#include <KDGpu/api/api_swapchain.h>

#include <KDGpu/gpu_core.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct Device_t;

struct KDGPU_EXPORT VulkanSwapchain : public ApiSwapchain {
    explicit VulkanSwapchain(VkSwapchainKHR _swapchain,
                             Format _format,
                             Extent3D _extent,
                             uint32_t _arrayLayers,
                             TextureUsageFlags _imageUsageFlags,
                             VulkanResourceManager *_vulkanResourceManager,
                             const Handle<Device_t> &_deviceHandle);

    std::vector<Handle<Texture_t>> getTextures() final;

    AcquireImageResult getNextImageIndex(uint32_t &imageIndex, const Handle<GpuSemaphore_t> &semaphore) final;

    VkSwapchainKHR swapchain{ VK_NULL_HANDLE };
    Format format;
    Extent3D extent;
    uint32_t arrayLayers;
    TextureUsageFlags imageUsageFlags;
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
