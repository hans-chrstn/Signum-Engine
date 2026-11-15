#include "application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/logging/log_categories.hpp"
#include "engine/core/logging/log_level.hpp"
#include "engine/core/logging/logger_factory.hpp"
#include "engine/core/time/frame_clock.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/graphics_backend.hpp"
#include "engine/renderer/graphics_submission_progress.hpp"
#include "engine/renderer/renderer_creation_info.hpp"
#include "engine/renderer/renderer_factory.hpp"
#include "engine/renderer/renderer_interface.hpp"
#include "engine/renderer/vulkan/vulkan_renderer.hpp"
#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace SNE::Engine::Core {
    namespace {
        [[nodiscard]] auto
        validateApplicationConfiguration(ApplicationConfiguration configuration)
            -> ApplicationConfiguration {
            if (configuration.graphics_backend !=
                Renderer::GraphicsBackend::Vulkan) {
                Assertion::failAssertion(
                    Assertion::AssertionType::Precondition,
                    Error::Subsystem::Renderer,
                    "Application requires a recognized graphics backend");
            }

            return configuration;
        }
    } // namespace

    Application::Application(ApplicationConfiguration configuration)
        : m_Configuration(
              validateApplicationConfiguration(std::move(configuration))),
          m_Logger(Logging::makeLogger(m_Configuration.logging)),
          m_GlfwContext(&m_Logger),
          m_Window(m_Configuration.initial_window_size,
                   m_Configuration.application_name),
          m_Renderer(Renderer::createRenderer(Renderer::RendererCreationInfo{
              .backend = m_Configuration.graphics_backend,
              .application_name = m_Configuration.application_name,
              .window = m_Window,
              .presentation_preference =
                  m_Configuration.presentation_preference,
              .development_diagnostics_enabled =
                  m_Configuration.development_diagnostics_enabled,
              .initial_mesh = m_Configuration.initial_mesh,
              .initial_texture = m_Configuration.initial_texture,
              .initial_camera = m_Configuration.initial_camera,
              .initial_graphics_shaders =
                  std::move(m_Configuration.initial_graphics_shaders),
              .initial_buffer_bindings =
                  m_Configuration.initial_buffer_bindings,
              .initial_texture_bindings =
                  m_Configuration.initial_texture_bindings,
              .require_swapchain_transfer_source = false,
              .logger = &m_Logger,
          })) {
        m_Logger.log(Logging::LogLevel::Info, Logging::Categories::Core,
                     "Application Initialized");
    }

    auto Application::run(
        const std::function<void(Application &)> &on_update,
        const Renderer::Vulkan::VulkanFrameCommandRecorder
            &record_external_commands,
        const std::function<void(std::uint64_t)> &on_graphics_submission,
        const std::function<void(Application &,
                                 std::span<const Platform::PlatformEvent>)>
            &on_platform_events) -> void {
        Renderer::Vulkan::VulkanRenderer *renderer = nullptr;

        if (record_external_commands) {
            renderer = dynamic_cast<Renderer::Vulkan::VulkanRenderer *>(
                m_Renderer.get());

            if (renderer == nullptr) {
                Assertion::failAssertion(
                    Assertion::AssertionType::Precondition,
                    Error::Subsystem::Renderer,
                    "Application::run external Vulkan command "
                    "recording requires a Vulkan renderer");
            }
        }

        while (!m_Window.shouldClose()) {
            m_FrameDelta = m_FrameClock.sample(Time::Clock::now());
            m_InputState.beginFrame();
            Platform::Window::pollEvents();

            std::vector<Platform::PlatformEvent> events = m_Window.takeEvents();

            m_InputState.applyEvents(
                std::span<const Platform::PlatformEvent>{events});

            if (on_platform_events && !events.empty()) {
                on_platform_events(
                    *this, std::span<const Platform::PlatformEvent>{events});
            }

            if (on_update) {
                on_update(*this);
            }

            const std::uint64_t previous_submission_id =
                m_Renderer->graphicsSubmissionProgress().last_submitted;

            if (!record_external_commands) {
                m_Renderer->renderFrame();
            } else {
                renderer->renderFrame(record_external_commands);
            }

            const std::uint64_t current_submission_id =
                m_Renderer->graphicsSubmissionProgress().last_submitted;

            if (on_graphics_submission &&
                previous_submission_id != current_submission_id) {
                on_graphics_submission(current_submission_id);
            }
        }
    }

    auto Application::window() const noexcept -> const Platform::Window & {
        return m_Window;
    }

    auto Application::renderer() noexcept -> Renderer::IRenderer & {
        return *m_Renderer;
    }

    auto Application::renderer() const noexcept -> const Renderer::IRenderer & {
        return *m_Renderer;
    }

    auto
    Application::setDrawInstanceModelMatrix(std::size_t instance_index,
                                            const Math::Matrix4f &model_matrix)
        -> void {
        m_Renderer->setDrawInstanceModelMatrix(instance_index, model_matrix);
    }

    auto Application::setCameraViewMatrix(const Math::Matrix4f &view_matrix)
        -> void {
        m_Renderer->setCameraViewMatrix(view_matrix);
    }

    auto Application::setSceneViewportExtent(
        std::optional<Renderer::RenderTargetExtent> extent) -> void {
        m_Renderer->setSceneViewportExtent(extent);
    }

    auto Application::graphicsSubmissionProgress() const noexcept
        -> Renderer::GraphicsSubmissionProgress {
        return m_Renderer->graphicsSubmissionProgress();
    }

    auto
    Application::updateMaterialBuffer(Renderer::MaterialBufferUpdateInput input)
        -> void {
        m_Renderer->updateMaterialBuffer(input);
    }

    auto Application::replaceFragmentShader(
        Renderer::Shader::ShaderStageInput fragment_shader) -> void {
        m_Renderer->replaceFragmentShader(std::move(fragment_shader));
    }

    auto Application::setCursorCaptured(bool captured) -> void {
        m_Window.setCursorCaptured(captured);
    }

    auto Application::requestClose() -> void {
        m_Window.requestClose();
    }

    auto
    Application::registerMesh(const Renderer::MeshUploadData &mesh,
                              const Renderer::VertexLayoutDescription &layout)
        -> Renderer::GpuResourceHandle {
        return m_Renderer->registerMesh(mesh, layout);
    }

    auto Application::registerMaterialConfiguration(
        Renderer::MaterialDescription description)
        -> Renderer::GpuResourceHandle {
        return m_Renderer->registerMaterialConfiguration(
            std::move(description));
    }

    auto Application::setDrawInstances(
        std::span<const Renderer::MeshDrawInstance> instances) -> void {
        m_Renderer->setDrawInstances(instances);
    }

    auto Application::drawInstances() const
        -> std::vector<Renderer::MeshDrawInstance> {
        return m_Renderer->drawInstances();
    }

    auto Application::inputState() const noexcept
        -> const Platform::InputState & {
        return m_InputState;
    }

    auto Application::frameDelta() const noexcept -> Time::Duration {
        return m_FrameDelta;
    }
} // namespace SNE::Engine::Core
