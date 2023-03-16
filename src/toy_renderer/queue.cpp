#include "queue.h"

#include <toy_renderer/graphics_api.h>
#include <toy_renderer/queue.h>
#include <toy_renderer/resource_manager.h>
#include <toy_renderer/api/api_queue.h>

namespace ToyRenderer {

Queue::Queue()
{
}

Queue::Queue(GraphicsApi *api, const QueueDescription &description)
    : m_api(api)
    , m_queue(description.queue)
    , m_flags(description.flags)
    , m_timestampValidBits(description.timestampValidBits)
    , m_minImageTransferGranularity(description.minImageTransferGranularity)
    , m_queueTypeIndex(description.queueTypeIndex)
{
}

Queue::~Queue()
{
}

void Queue::submit(const SubmitOptions &options)
{
    auto apiQueue = m_api->resourceManager()->getQueue(m_queue);
    apiQueue->submit(options);
}

std::vector<PresentResult> Queue::present(const PresentOptions &options)
{
    auto apiQueue = m_api->resourceManager()->getQueue(m_queue);
    return apiQueue->present(options);
}

} // namespace ToyRenderer
