#include "vulkan_vertex_input.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/vulkan/geometry/vulkan_vertex_format.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto
    makeVulkanVertexInputDescriptions(const VertexLayoutDescription &layout,
                                      std::uint32_t vertex_stride)
        -> VulkanVertexInputDescriptions {
        if (!layout.attributes.empty() && vertex_stride == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan vertex input requires a nonzero stride when attributes "
                "are present");
        }

        VulkanVertexInputDescriptions vertex_description{};
        vertex_description.binding.binding = 0U;
        vertex_description.binding.stride = vertex_stride;
        vertex_description.binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        vertex_description.attributes.reserve(layout.attributes.size());

        for (const VertexAttributeDescription &attribute : layout.attributes) {
            const std::uint32_t attribute_size =
                vertexFormatSizeInBytes(attribute.format);

            if (attribute_size > vertex_stride) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Vulkan vertex attribute size exceeds vertex stride");
            }

            if (attribute.offset > (vertex_stride - attribute_size)) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Vulkan vertex attribute extends beyond vertex stride");
            }

            for (const VkVertexInputAttributeDescription &vertex_attribute :
                 vertex_description.attributes) {
                if (vertex_attribute.location == attribute.location) {
                    Core::Assertion::failAssertion(
                        Core::Assertion::AssertionType::Precondition,
                        Core::Error::Subsystem::Vulkan,
                        "Vulkan vertex input requires unique attribute "
                        "locations");
                }
            }

            VkVertexInputAttributeDescription attribute_description{};
            attribute_description.binding = 0U;
            attribute_description.location = attribute.location;
            attribute_description.format = toVkFormat(attribute.format);
            attribute_description.offset = attribute.offset;

            vertex_description.attributes.push_back(attribute_description);
        }

        return vertex_description;
    }
} // namespace SNE::Engine::Renderer::Vulkan
