#include <string_view>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    [[nodiscard]] auto toString(VkResult result) noexcept -> std::string_view;
}
