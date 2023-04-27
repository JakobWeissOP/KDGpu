/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "vulkan_surface.h"

namespace KDGpu {

VulkanSurface::VulkanSurface(VkSurfaceKHR _surface, VkInstance _instance, bool _isOwned)
    : ApiSurface()
    , surface(_surface)
    , instance(_instance)
    , isOwned(_isOwned)
{
}

} // namespace KDGpu
