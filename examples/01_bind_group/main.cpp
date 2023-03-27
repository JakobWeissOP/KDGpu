#include <kdgpu/device.h>
#include <kdgpu/instance.h>
#include <kdgpu/bind_group_description.h>
#include <kdgpu/bind_group.h>
#include <kdgpu/bind_group_options.h>
#include <kdgpu/bind_group_layout.h>
#include <kdgpu/bind_group_layout_options.h>
#include <kdgpu/graphics_pipeline.h>
#include <kdgpu/graphics_pipeline_options.h>

#include <kdgpu/vulkan/vulkan_graphics_api.h>

#include <kdgpu_kdgui/view.h>

#include <KDGui/gui_application.h>

#include <fstream>
#include <iostream>
#include <map>
#include <span>
#include <vector>

using namespace KDGui;
using namespace KDGpu;
using namespace KDGpuKDGui;

inline std::string assetPath()
{
#if defined(KDGPU_ASSET_PATH)
    return KDGPU_ASSET_PATH;
#else
    return "";
#endif
}

class Pipeline;

enum class IndexFormat {
    UINT16 = 0,
    UINT32 = 1
};

class RenderPass
{
public:
    // void begin();
    // void end();

    void setBindGroup(uint32_t groupSlot, const Handle<BindGroup> &bindGroup) { }
    void setPipeline(const Handle<Pipeline> &pipeline) { }
    void setVertexBuffer(uint32_t index, const Handle<Buffer> &buffer, uint64_t offset, uint64_t size) { }
    void setIndexBuffer(const Handle<Buffer> &buffer, IndexFormat format, uint64_t offset, uint64_t size) { }
    void draw(uint32_t drawCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) { }

    // TODO: Add drawIndexed()
};

// Data maintained by the renderer
Handle<BindGroup> frameBindGroup;
Handle<BindGroup> instancesBindGroup;

struct Material {
    Handle<BindGroup> bindGroup;
};

struct GpuPrimitive {
    std::vector<UniformBufferBinding> vertexBuffers;

    uint32_t drawCount;
    struct Instances {
        uint32_t count;
        uint32_t first;
    } instances;
};

using GpuPrimitives = std::vector<GpuPrimitive>;

struct GpuPipeline {
    Handle<Pipeline> pipeline;
    std::map<Material, GpuPrimitives> materialPrimitives;
};

std::vector<GpuPipeline> gpuPipelineData;

// Pseudo-code for a render loop
void renderGltf(RenderPass &renderPass)
{
    renderPass.setBindGroup(0, frameBindGroup);
    renderPass.setBindGroup(1, instancesBindGroup);

    for (const auto &gpuPipeline : gpuPipelineData) {
        renderPass.setPipeline(gpuPipeline.pipeline);

        // Loop through every material that uses this pipeline and get an array of primitives
        // that uses that material.
        for (const auto [material, primitives] : gpuPipeline.materialPrimitives) {
            // Set the material bind group.
            renderPass.setBindGroup(2, material.bindGroup);

            // Loop through the primitives that use the current material/pipeline combo and draw
            // them as usual.
            for (const auto &gpuPrimitive : primitives) {
                for (const auto &vertexBuffer : gpuPrimitive.vertexBuffers) {
                    // renderPass.setVertexBuffer(0, vertexBuffer.buffer, vertexBuffer.offset, vertexBuffer.size);
                }

                renderPass.draw(gpuPrimitive.drawCount, gpuPrimitive.instances.count, 0, gpuPrimitive.instances.first);
            }
        }
    }
}

int main()
{
    GuiApplication app;

    // Initiate Vulkan API
    std::unique_ptr<GraphicsApi> api = std::make_unique<VulkanGraphicsApi>();

    // Create Instance
    InstanceOptions instanceOptions = {
        .applicationName = "02_hello_triangle",
        .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
    };
    Instance instance = api->createInstance(instanceOptions);

    // Rendering Surface
    View v;
    const SurfaceOptions surfaceOptions = View::surfaceOptions(&v);
    Surface s = instance.createSurface(surfaceOptions);

    // Select best device to render on surface
    AdapterAndDevice aAndD = instance.createDefaultDevice(s);

    Device device = std::move(aAndD.device);

    // Our shader uniform layout
    const BindGroupLayoutOptions bindGroupLayoutOptions = {
        .bindings = {
                { .binding = 0,
                  .count = 1,
                  .resourceType = ResourceBindingType::CombinedImageSampler,
                  .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) },
                { .binding = 1,
                  .count = 1,
                  .resourceType = ResourceBindingType::CombinedImageSampler,
                  .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) },
                { .binding = 2,
                  .count = 1,
                  .resourceType = ResourceBindingType::CombinedImageSampler,
                  .shaderStages = ShaderStageFlags(ShaderStageFlagBits::FragmentBit) },
                { .binding = 3,
                  .count = 1,
                  .resourceType = ResourceBindingType::UniformBuffer,
                  .shaderStages = ShaderStageFlags(ShaderStageFlagBits::VertexBit) },
        }
    };

    const BindGroupLayout bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutOptions);

    const PipelineLayoutOptions pipelineLayoutOptions = {
        .bindGroupLayouts = { bindGroupLayout }
    };

    const PipelineLayout pipelineLayout = device.createPipelineLayout(pipelineLayoutOptions);

    // Create a vertex shader and fragment shader (spir-v only for now)
    const auto vertexShaderPath = assetPath() + "/shaders/examples/02_hello_triangle/hello_triangle.vert.spv";
    auto vertexShader = device.createShaderModule(readShaderFile(vertexShaderPath));

    const auto fragmentShaderPath = assetPath() + "/shaders/examples/02_hello_triangle/hello_triangle.frag.spv";
    auto fragmentShader = device.createShaderModule(readShaderFile(fragmentShaderPath));

    // Create a pipeline
    // clang-format off
    GraphicsPipelineOptions pipelineOptions = {
        .shaderStages = {
            { .shaderModule = vertexShader.handle(), .stage = ShaderStageFlagBits::VertexBit },
            { .shaderModule = fragmentShader.handle(), .stage = ShaderStageFlagBits::FragmentBit }
        },
        .layout = pipelineLayout.handle(),
        .vertex = {
            .buffers = {
                { .binding = 0, .stride = 2 * 4 * sizeof(float) }
            },
            .attributes = {
                { .location = 0, .binding = 0, .format = Format::R32G32B32A32_SFLOAT }, // Position
                { .location = 1, .binding = 0, .format = Format::R32G32B32A32_SFLOAT, .offset = 4 * sizeof(float) } // Color
            }
        },
        .renderTargets = {
            { .format = Format::R8G8B8A8_UNORM }
        },
        .depthStencil = {
            .format = Format::D24_UNORM_S8_UINT,
            .depthWritesEnabled = true,
            .depthCompareOperation = CompareOperation::Less
        }
    };
    // clang-format on
    GraphicsPipeline pipeline = device.createGraphicsPipeline(pipelineOptions);

    // Assume we have some textures and a buffer for a material and we want to bind these...
    Handle<TextureView_t> textureBaseColor;
    Handle<TextureView_t> textureMetalRough;
    Handle<TextureView_t> textureNormal;
    Handle<Sampler_t> sampler;
    Handle<Buffer_t> materialUniforms;

    // BindGroupOptions to create a bind group that sets data for above layout
    BindGroupOptions bindGroupOptions = { .layout = bindGroupLayout,
                                          .resources = {
                                                  { .binding = 0,
                                                    .resource = TextureViewBinding{ .textureView = textureBaseColor /*, .sampler = sampler */ } },
                                                  { .binding = 1,
                                                    .resource = TextureViewBinding{ .textureView = textureMetalRough /*, .sampler = sampler */ } },
                                                  { .binding = 2,
                                                    .resource = TextureViewBinding{ .textureView = textureNormal /*, .sampler = sampler */ } },
                                                  { .binding = 3,
                                                    .resource = UniformBufferBinding{ .buffer = materialUniforms } },
                                          } };

    // Create the bind group
    BindGroup bindGroup = device.createBindGroup(bindGroupOptions);

    // And release it again
    // resourceManager->deleteBindGroup(bindGroup);

    return 0;
}
