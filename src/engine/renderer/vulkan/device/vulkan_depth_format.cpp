#include "vulkan_depth_format.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <array>
#include <cstddef>
#include <optional>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    auto
    selectPreferredDepthFormat(std::span<const DepthFormatCandidate> candidates)
        -> std::optional<VkFormat> {
        for (const DepthFormatCandidate &candidate : candidates) {
            if ((candidate.optimal_tiling_features &
                 VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0U) {
                return candidate.format;
            }
        }

        return std::nullopt;
    }

    auto selectSupportedDepthFormat(VkPhysicalDevice physical_device)
        -> std::optional<VkFormat> {
        if (physical_device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Depth-format selection requires a valid Vulkan physical "
                "device");
        }

        constexpr std::array<VkFormat, 2> formats{
            VK_FORMAT_D32_SFLOAT,
            VK_FORMAT_D16_UNORM,
        };

        std::array<DepthFormatCandidate, formats.size()> candidates{};

        for (std::size_t index{}; index < formats.size(); ++index) {
            const VkFormat format = formats[index];

            VkFormatProperties properties{};
            vkGetPhysicalDeviceFormatProperties(physical_device, format,
                                                &properties);

            candidates[index] = DepthFormatCandidate{
                .format = format,
                .optimal_tiling_features = properties.optimalTilingFeatures,
            };
        }

        return selectPreferredDepthFormat(candidates);
    }
} // namespace SNE::Engine::Renderer::Vulkan
