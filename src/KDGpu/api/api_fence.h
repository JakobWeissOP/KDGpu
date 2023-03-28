#pragma once

#include <KDGpu/gpu_core.h>

namespace KDGpu {

struct ApiFence {
    virtual void wait() = 0;
    virtual void reset() = 0;
    virtual FenceStatus status() = 0;
};

} // namespace KDGpu
