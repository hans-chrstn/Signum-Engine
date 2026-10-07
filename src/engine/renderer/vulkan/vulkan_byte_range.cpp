#include "vulkan_byte_range.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto ByteRange::fitsWithin(VkDeviceSize total_size) const noexcept -> bool {
        if (offset > total_size) {
            return false;
        }

        return total_size - offset >= size;
    }
} // namespace SNE::Engine::Renderer::Vulkan
