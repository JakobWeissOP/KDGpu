#pragma once

#include <KDGpu/api/api_compute_pass_command_recorder.h>
#include <KDGpu/handle.h>
#include <vulkan/vulkan.h>

namespace KDGpu {

class VulkanResourceManager;

struct ComputePipeline_t;
struct Device_t;

struct VulkanComputePassCommandRecorder : public ApiComputePassCommandRecorder {

    explicit VulkanComputePassCommandRecorder(VkCommandBuffer _commandBuffer,
                                              VulkanResourceManager *_vulkanResourceManager,
                                              const Handle<Device_t> &_deviceHandle);

    void setPipeline(const Handle<ComputePipeline_t> &pipeline) final;
    void setBindGroup(uint32_t group, const Handle<BindGroup_t> &bindGroup,
                      const Handle<PipelineLayout_t> &pipelineLayout, const std::vector<uint32_t> &dynamicBufferOffsets) final;
    void dispatchCompute(const ComputeCommand &command) final;
    void dispatchCompute(const std::vector<ComputeCommand> &commands) final;
    void dispatchComputeIndirect(const ComputeCommandIndirect &command) final;
    void dispatchComputeIndirect(const std::vector<ComputeCommandIndirect> &commands) final;
    void pushConstant(const PushConstantRange &constantRange, const void *data) final;
    void end() final;

    VkCommandBuffer commandBuffer{ VK_NULL_HANDLE };
    VulkanResourceManager *vulkanResourceManager{ nullptr };
    Handle<Device_t> deviceHandle;
    Handle<ComputePipeline_t> pipeline;
};

} // namespace KDGpu