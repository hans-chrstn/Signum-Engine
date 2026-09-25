#include <string_view>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Converts a Vulkan result code to its symbolic name.
     *
     * Known VkResult values are represented using their Vulkan identifier.
     * Unrecognized result values produce a generic unknown-result string.
     *
     * @param result Vulkan result code to convert.
     * @return Symbolic string representation of the result.
     */
    [[nodiscard]] auto toString(VkResult result) noexcept -> std::string_view;
} // namespace SNE::Engine::Renderer::Vulkan
