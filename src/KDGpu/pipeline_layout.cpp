/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "pipeline_layout.h"
#include <KDGpu/graphics_api.h>
#include <KDGpu/resource_manager.h>
#include <KDGpu/api/api_pipeline_layout.h>

namespace KDGpu {

PipelineLayout::PipelineLayout() = default;

PipelineLayout::PipelineLayout(GraphicsApi *api,
                               const Handle<Device_t> &device,
                               const PipelineLayoutOptions &options)
    : m_api(api)
    , m_device(device)
    , m_pipelineLayout(m_api->resourceManager()->createPipelineLayout(m_device, options))
{
}

PipelineLayout::PipelineLayout(PipelineLayout &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_pipelineLayout = other.m_pipelineLayout;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_pipelineLayout = {};
}

PipelineLayout &PipelineLayout::operator=(PipelineLayout &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deletePipelineLayout(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_pipelineLayout = other.m_pipelineLayout;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_pipelineLayout = {};
    }
    return *this;
}

PipelineLayout::~PipelineLayout()
{
    if (isValid())
        m_api->resourceManager()->deletePipelineLayout(handle());
}

bool operator==(const PipelineLayout &a, const PipelineLayout &b)
{
    return a.m_api == b.m_api && a.m_device == b.m_device && a.m_pipelineLayout == b.m_pipelineLayout;
}

bool operator!=(const PipelineLayout &a, const PipelineLayout &b)
{
    return !(a == b);
}

} // namespace KDGpu
