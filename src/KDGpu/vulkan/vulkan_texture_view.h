/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#pragma once

#include <KDGpu/api/api_texture_view.h>
#include <KDGpu/handle.h>
#include <KDGpu/kdgpu_export.h>

#include <vulkan/vulkan.h>

namespace KDGpu {

struct Texture_t;
struct Device_t;

struct KDGPU_EXPORT VulkanTextureView : public ApiTextureView {
    explicit VulkanTextureView(VkImageView _imageView,
                               const Handle<Texture_t> &_textureHandle,
                               const Handle<Device_t> &_deviceHandle);

    VkImageView imageView{ VK_NULL_HANDLE };
    Handle<Texture_t> textureHandle;
    Handle<Device_t> deviceHandle;
};

} // namespace KDGpu
