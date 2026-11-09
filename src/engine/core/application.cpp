#include "application.hpp"
#include "engine/core/logging/log_level.hpp"
#include "engine/core/logging/stream_log_sink.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include "engine/renderer/vulkan/vulkan_graphics_submission_progress.hpp"
#include "engine/renderer/vulkan/vulkan_scene_viewport_image_info.hpp"
#include <cstdint>
#include <iostream>
#include <memory>
#include <utility>

namespace SNE::Engine::Core {
    Application::Application(ApplicationConfiguration configuration)
        : m_Configuration(std::move(configuration)),
          m_Logger(std::make_unique<Logging::StreamLogSink>(std::clog)),
          m_Window(m_Configuration.initial_window_size,
                   m_Configuration.application_name),
          m_Renderer(m_Configuration.application_name, m_Window,
                     m_Configuration.presentation_preference,
                     m_Configuration.development_diagnostics_enabled,
                     m_Configuration.initial_mesh,
                     m_Configuration.initial_texture,
                     m_Configuration.initial_camera,
                     std::move(m_Configuration.initial_graphics_shaders),
                     m_Configuration.initial_buffer_bindings,
                     m_Configuration.initial_texture_bindings) {
        m_Logger.log(Logging::LogLevel::Info, "Core",
                     "Application Initialized");
    }

    auto Application::run(
        const std::function<void(Application &)> &on_update,
        const Renderer::Vulkan::VulkanFrameCommandRecorder
            &record_external_commands,
        const std::function<void(std::uint64_t)> &on_graphics_submission)
        -> void {
        while (!m_Window.shouldClose()) {
            Platform::Window::pollEvents();

            if (on_update) {
                on_update(*this);
            }

            const std::uint64_t previous_submission_id =
                m_Renderer.graphicsSubmissionProgress().last_submitted;

            m_Renderer.renderFrame(record_external_commands);

            const std::uint64_t current_submission_id =
                m_Renderer.graphicsSubmissionProgress().last_submitted;

            if (on_graphics_submission &&
                previous_submission_id != current_submission_id) {
                on_graphics_submission(current_submission_id);
            }
        }
    }

    auto Application::window() const noexcept -> const Platform::Window & {
        return m_Window;
    }

    auto
    Application::setDrawInstanceModelMatrix(std::size_t instance_index,
                                            const Math::Matrix4f &model_matrix)
        -> void {
        m_Renderer.setDrawInstanceModelMatrix(instance_index, model_matrix);
    }

    auto Application::setCameraViewMatrix(const Math::Matrix4f &view_matrix)
        -> void {
        m_Renderer.setCameraViewMatrix(view_matrix);
    }

    auto Application::setSceneViewportExtent(
        std::optional<Renderer::RenderTargetExtent> extent) -> void {
        m_Renderer.setSceneViewportExtent(extent);
    }

    auto Application::sceneViewportImageInfo() const
        -> std::optional<Renderer::Vulkan::VulkanSceneViewportImageInfo> {
        return m_Renderer.sceneViewportImageInfo();
    }

    auto Application::graphicsSubmissionProgress() const noexcept
        -> Renderer::Vulkan::VulkanGraphicsSubmissionProgress {
        return m_Renderer.graphicsSubmissionProgress();
    }

    auto
    Application::updateMaterialBuffer(Renderer::MaterialBufferUpdateInput input)
        -> void {
        m_Renderer.updateMaterialBuffer(input.set, input.binding,
                                        input.array_element, input.data);
    }

    auto Application::replaceFragmentShader(
        Renderer::Shader::ShaderStageInput fragment_shader) -> void {
        m_Renderer.replaceFragmentShader(std::move(fragment_shader));
    }

    auto Application::setCursorCaptured(bool captured) -> void {
        m_Window.setCursorCaptured(captured);
    }

    auto Application::externalRenderingInfo() const
        -> Renderer::Vulkan::VulkanExternalRenderingInfo {
        return m_Renderer.externalRenderingInfo();
    }

    auto Application::requestClose() -> void {
        m_Window.requestClose();
    }

    auto
    Application::registerMesh(const Renderer::MeshUploadData &mesh,
                              const Renderer::VertexLayoutDescription &layout)
        -> Renderer::GpuResourceHandle {
        return m_Renderer.registerMesh(mesh, layout);
    }

    auto Application::registerMaterialConfiguration(
        Renderer::MaterialDescription description)
        -> Renderer::GpuResourceHandle {
        return m_Renderer.registerMaterialConfiguration(std::move(description));
    }

    auto Application::setDrawInstances(
        std::span<const Renderer::MeshDrawInstance> instances) -> void {
        m_Renderer.setDrawInstances(instances);
    }

    auto Application::drawInstances() const
        -> std::vector<Renderer::MeshDrawInstance> {
        return m_Renderer.drawInstances();
    }
} // namespace SNE::Engine::Core
