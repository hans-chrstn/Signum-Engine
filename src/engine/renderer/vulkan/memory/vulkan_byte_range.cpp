#include "vulkan_byte_range.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto ByteRange::fitsWithin(VkDeviceSize total_size) const noexcept -> bool {
        if (offset > total_size) {
            return false;
        }

        return total_size - offset >= size;
    }

    auto ByteRange::overlaps(const ByteRange &other) const noexcept -> bool {
        if (this->size == 0U) {
            return false;
        }

        if (other.size == 0U) {
            return false;
        }

        if (this->offset <= other.offset) {
            return (other.offset - this->offset) < this->size;
        }

        return (this->offset - other.offset) < other.size;
    }
} // namespace SNE::Engine::Renderer::Vulkan
