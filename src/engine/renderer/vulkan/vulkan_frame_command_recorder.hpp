#pragma once

#include <functional>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Callback used to record external Vulkan graphics commands.
     *
     * Receives a borrowed command buffer while dynamic rendering is active.
     * The callback may record compatible graphics commands but must not
     * end rendering, reset or submit the command buffer, or retain its handle.
     *
     * An empty callback indicates that no external commands are recorded.
     *
     * @note Callbacks are invoked synchronously and must not throw exceptions.
     */
    using VulkanFrameCommandRecorder = std::function<void(VkCommandBuffer)>;
} // namespace SNE::Engine::Renderer::Vulkan
