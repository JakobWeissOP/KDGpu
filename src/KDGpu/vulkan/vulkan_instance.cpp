/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_instance.h"

#include <KDGpu/vulkan/vulkan_adapter.h>
#include <KDGpu/vulkan/vulkan_resource_manager.h>

#include <KDGpu/utils/logging.h>

#if defined(KDGPU_PLATFORM_WIN32)
#include <vulkan/vulkan_win32.h>
#endif

#if defined(KDGPU_PLATFORM_LINUX)
#include <vulkan/vulkan_xcb.h>
#endif

#if defined(KDGPU_PLATFORM_MACOS)
extern VkSurfaceKHR createVulkanSurface(VkInstance instance, const KDGpu::SurfaceOptions &options);
#endif

namespace KDGpu {

VulkanInstance::VulkanInstance(VulkanResourceManager *_vulkanResourceManager, VkInstance _instance, bool _isOwned) noexcept
    : ApiInstance()
    , vulkanResourceManager(_vulkanResourceManager)
    , instance(_instance)
    , isOwned(_isOwned)
{
}

std::vector<Extension> VulkanInstance::extensions() const
{
    uint32_t extensionCount{ 0 };
    if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Unable to enumerate instance extensions");
        return {};
    }

    std::vector<VkExtensionProperties> vkExtensions(extensionCount);
    if (vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, vkExtensions.data()) != VK_SUCCESS) {
        SPDLOG_LOGGER_CRITICAL(Logger::logger(), "Unable to query instance extensions");
        return {};
    }

    std::vector<Extension> extensions;
    extensions.reserve(extensionCount);
    for (const auto &vkExtension : vkExtensions) {
        extensions.emplace_back(Extension{
                .name = vkExtension.extensionName,
                .version = vkExtension.specVersion });
    }

    return extensions;
}

std::vector<Handle<Adapter_t>> VulkanInstance::queryAdapters(const Handle<Instance_t> &instanceHandle)
{
    // Query the physical devices from the instance
    uint32_t adapterCount = 0;
    vkEnumeratePhysicalDevices(instance, &adapterCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(adapterCount);
    vkEnumeratePhysicalDevices(instance, &adapterCount, physicalDevices.data());

    // Store the resulting physical devices in the resource manager so that
    // the Adapters can access them later, and create the Adapters.
    std::vector<Handle<Adapter_t>> adapterHandles;
    adapterHandles.reserve(adapterCount);
    for (uint32_t adapterIndex = 0; adapterIndex < adapterCount; ++adapterIndex) {
        VulkanAdapter vulkanAdapter{ physicalDevices[adapterIndex], vulkanResourceManager, instanceHandle };
        adapterHandles.emplace_back(vulkanResourceManager->insertAdapter(vulkanAdapter));
    }

    return adapterHandles;
}

Handle<Surface_t> VulkanInstance::createSurface(const SurfaceOptions &options)
{
    VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };
#if defined(KDGPU_PLATFORM_WIN32)
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR{ nullptr };
    vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR"));
    if (!vkCreateWin32SurfaceKHR)
        return {};

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = GetModuleHandle(nullptr);
    createInfo.hwnd = options.hWnd;

    if (vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &vkSurface) != VK_SUCCESS)
        return {};
#endif

#if defined(KDGPU_PLATFORM_LINUX)
    PFN_vkCreateXcbSurfaceKHR vkCreateXcbSurfaceKHR{ nullptr };
    vkCreateXcbSurfaceKHR = (PFN_vkCreateXcbSurfaceKHR)vkGetInstanceProcAddr(instance, "vkCreateXcbSurfaceKHR");
    if (!vkCreateXcbSurfaceKHR)
        return {};

    VkXcbSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.connection = options.connection;
    createInfo.window = options.window;

    if (vkCreateXcbSurfaceKHR(instance, &createInfo, nullptr, &vkSurface) != VK_SUCCESS)
        return {};
#endif

#if defined(KDGPU_PLATFORM_MACOS)
    vkSurface = createVulkanSurface(instance, options);
#endif

    VulkanSurface vulkanSurface(vkSurface, instance);
    auto surfaceHandle = vulkanResourceManager->insertSurface(vulkanSurface);
    return surfaceHandle;
}

Handle<Surface_t> VulkanInstance::createSurface(VkSurfaceKHR vkSurface)
{
    VulkanSurface vulkanSurface(vkSurface, instance, false);
    auto surfaceHandle = vulkanResourceManager->insertSurface(vulkanSurface);
    return surfaceHandle;
}

} // namespace KDGpu
