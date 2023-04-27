/*
  This file is part of KDGpu.

  SPDX-FileCopyrightText: 2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>

  SPDX-License-Identifier: MIT

  Contact KDAB at <info@kdab.com> for commercial licensing options.
*/

#include "example_engine_layer.h"

#include <KDGpuExample/engine.h>
#include <KDGpuExample/imgui_input_handler.h>
#include <KDGpuExample/imgui_item.h>
#include <KDGpuExample/imgui_renderer.h>

#include <KDGpu/buffer_options.h>
#include <KDGpu/swapchain_options.h>
#include <KDGpu/texture_options.h>

#include <KDGui/gui_application.h>

#include <imgui.h>

#include <algorithm>

namespace KDGpuExample {

ExampleEngineLayer::ExampleEngineLayer()
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
{
}

ExampleEngineLayer::ExampleEngineLayer(const SampleCountFlagBits samples)
    : EngineLayer()
    , m_api(std::make_unique<VulkanGraphicsApi>())
    , m_samples(samples)
{
}

ExampleEngineLayer::~ExampleEngineLayer()
{
}

void ExampleEngineLayer::recreateSwapChain()
{
    // Create a swapchain of images that we will render to.
    SwapchainOptions swapchainOptions = {
        .surface = m_surface,
        .format = m_swapchainFormat,
        .imageExtent = { .width = m_window->width(), .height = m_window->height() },
        .presentMode = m_presentMode,
        .oldSwapchain = m_swapchain,
    };

    // Create swapchain and destroy previous one implicitly
    m_swapchain = m_device.createSwapchain(swapchainOptions);

    const auto &swapchainTextures = m_swapchain.textures();
    const auto swapchainTextureCount = swapchainTextures.size();

    m_swapchainViews.clear();
    m_swapchainViews.reserve(swapchainTextureCount);
    for (uint32_t i = 0; i < swapchainTextureCount; ++i) {
        auto view = swapchainTextures[i].createView({ .format = swapchainOptions.format });
        m_swapchainViews.push_back(std::move(view));
    }

    // Create a depth texture to use for depth-correct rendering
    TextureOptions depthTextureOptions = {
        .type = TextureType::TextureType2D,
        .format = m_depthFormat,
        .extent = { m_window->width(), m_window->height(), 1 },
        .mipLevels = 1,
        .samples = m_samples,
        .usage = TextureUsageFlagBits::DepthStencilAttachmentBit,
        .memoryUsage = MemoryUsage::GpuOnly
    };
    m_depthTexture = m_device.createTexture(depthTextureOptions);
    m_depthTextureView = m_depthTexture.createView();
}

void ExampleEngineLayer::uploadBufferData(const BufferUploadOptions &options)
{
    m_stagingBuffers.emplace_back(m_queue.uploadBufferData(options));
}

void ExampleEngineLayer::uploadTextureData(const TextureUploadOptions &options)
{
    m_stagingBuffers.emplace_back(m_queue.uploadTextureData(options));
}

void ExampleEngineLayer::releaseStagingBuffers()
{
    // Loop over any staging buffers and see if the corresponding fence has been signalled.
    // If so, we can dispose of them
    const auto removedCount = std::erase_if(m_stagingBuffers, [](const UploadStagingBuffer &stagingBuffer) {
        return stagingBuffer.fence.status() == FenceStatus::Signalled;
    });
    if (removedCount)
        SPDLOG_INFO("Released {} staging buffers", removedCount);
}

void ExampleEngineLayer::drawImGuiOverlay(ImGuiContext *ctx)
{
    ImGui::SetCurrentContext(ctx);
    ImGui::SetNextWindowPos(ImVec2(10, 20));
    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin(
            "Basic Info",
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

    ImGui::Text("App: %s", KDGui::GuiApplication::instance()->applicationName().data());
    ImGui::Text("GPU: %s", m_device.adapter()->properties().deviceName.c_str());
    const auto fps = engine()->fps();
    ImGui::Text("%.2f ms/frame (%.1f fps)", (1000.0f / fps), fps);
    ImGui::End();

    for (const auto &func : m_imGuiOverlayDrawFunctions)
        func(ctx);
}

void ExampleEngineLayer::renderImGuiOverlay(RenderPassCommandRecorder *recorder, uint32_t inFlightIndex)
{
    // Updates the geometry buffers used by ImGui and records the commands needed to
    // get the ui into a render target.
    const Extent2D extent{ m_window->width(), m_window->height() };
    m_imguiOverlay->render(recorder, extent, inFlightIndex);
}

void ExampleEngineLayer::registerImGuiOverlayDrawFunction(const std::function<void(ImGuiContext *)> &func)
{
    m_imGuiOverlayDrawFunctions.push_back(func);
}

void ExampleEngineLayer::clearImGuiOverlayDrawFunctions()
{
    m_imGuiOverlayDrawFunctions.clear();
}

void ExampleEngineLayer::onAttached()
{
    m_window = std::make_unique<KDGpuKDGui::View>();
    m_window->title = KDGui::GuiApplication::instance()->applicationName();

    // Request an instance of the api with whatever layers and extensions we wish to request.
    InstanceOptions instanceOptions = {
        .applicationName = KDGui::GuiApplication::instance()->applicationName(),
        .applicationVersion = SERENITY_MAKE_API_VERSION(0, 1, 0, 0)
    };
    m_instance = m_api->createInstance(instanceOptions);

    // Create a drawable surface
    m_surface = m_window->createSurface(m_instance);

    // Create a device and a queue to use
    auto defaultDevice = m_instance.createDefaultDevice(m_surface);
    m_device = std::move(defaultDevice.device);
    m_queue = m_device.queues()[0];

    // Choose a presentation mode from the ones supported
    constexpr std::array<PresentMode, 4> preferredPresentModes = {
        PresentMode::Mailbox,
        PresentMode::FifoRelaxed,
        PresentMode::Fifo,
        PresentMode::Immediate
    };
    const auto &availableModes = defaultDevice.adapter->swapchainProperties(m_surface).presentModes;
    for (const auto &presentMode : preferredPresentModes) {
        const auto it = std::find(availableModes.begin(), availableModes.end(), presentMode);
        if (it != availableModes.end()) {
            m_presentMode = presentMode;
            break;
        }
    }

    // TODO: Move swapchain handling to View?
    recreateSwapChain();

    // Create the present complete and render complete semaphores
    for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        m_presentCompleteSemaphores[i] = m_device.createGpuSemaphore();
        m_renderCompleteSemaphores[i] = m_device.createGpuSemaphore();
    }

    // Create the ImGui overlay item
    m_imguiOverlay = std::make_unique<ImGuiItem>(&m_device, &m_queue);
    m_imguiOverlay->initialize(m_samples, m_swapchainFormat, m_depthFormat);

    initializeScene();
}

void ExampleEngineLayer::onDetached()
{
    m_imguiOverlay->cleanup();
    cleanupScene();

    m_imguiOverlay = {};

    m_presentCompleteSemaphores = {};
    m_renderCompleteSemaphores = {};
    m_depthTextureView = {};
    m_depthTexture = {};
    m_swapchainViews.clear();
    m_swapchain = {};
    m_queue = {};
    m_device = {};
    m_surface = {};
    m_instance = {};
    m_window = {};
}

void ExampleEngineLayer::update()
{
    ImGuiContext *context = m_imguiOverlay->context();
    ImGui::SetCurrentContext(context);

    // Set frame time and display size.
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = engine()->deltaTimeSeconds();
    io.DisplaySize = ImVec2(static_cast<float>(m_window->width()), static_cast<float>(m_window->height()));

    // Process the input events for ImGui
    m_imguiOverlay->updateInputState();

    // Call our imgui drawing function
    ImGui::NewFrame();
    drawImGuiOverlay(context);

    // TODO: Draw any additional registered imgui drawing callbacks

    // Process the ImGui drawing functions to generate geometry and commands. The actual buffers will be updated
    // and commands translated by the ImGuiRenderer later in the frame.
    ImGui::Render();
}

void ExampleEngineLayer::event(KDFoundation::EventReceiver *target, KDFoundation::Event *ev)
{
    // Forward window events to the ImGui overlay
    if (target == m_window.get())
        m_imguiOverlay->event(target, ev);
}

} // namespace KDGpuExample
