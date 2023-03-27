#include "bind_group_layout.h"

#include <kdgpu/graphics_api.h>
#include <kdgpu/resource_manager.h>
#include <kdgpu/api/api_bind_group_layout.h>

namespace KDGpu {

BindGroupLayout::BindGroupLayout() = default;
BindGroupLayout::~BindGroupLayout()
{
    if (isValid())
        m_api->resourceManager()->deleteBindGroupLayout(handle());
};

BindGroupLayout::BindGroupLayout(GraphicsApi *api,
                                 const Handle<Device_t> &device,
                                 const BindGroupLayoutOptions &options)
    : m_api(api)
    , m_device(device)
    , m_bindGroupLayout(m_api->resourceManager()->createBindGroupLayout(m_device, options))
{
}

BindGroupLayout::BindGroupLayout(BindGroupLayout &&other)
{
    m_api = other.m_api;
    m_device = other.m_device;
    m_bindGroupLayout = other.m_bindGroupLayout;

    other.m_api = nullptr;
    other.m_device = {};
    other.m_bindGroupLayout = {};
}

BindGroupLayout &BindGroupLayout::operator=(BindGroupLayout &&other)
{
    if (this != &other) {
        if (isValid())
            m_api->resourceManager()->deleteBindGroupLayout(handle());

        m_api = other.m_api;
        m_device = other.m_device;
        m_bindGroupLayout = other.m_bindGroupLayout;

        other.m_api = nullptr;
        other.m_device = {};
        other.m_bindGroupLayout = {};
    }
    return *this;
}

bool operator==(const BindGroupLayout &a, const BindGroupLayout &b)
{
    return (a.m_api == b.m_api && a.m_device == b.m_device && a.m_bindGroupLayout == b.m_bindGroupLayout);
}

bool operator!=(const BindGroupLayout &a, const BindGroupLayout &b)
{
    return !(a == b);
}

} // namespace KDGpu
