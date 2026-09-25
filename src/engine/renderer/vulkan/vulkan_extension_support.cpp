#include "vulkan_extension_support.hpp"
#include <cstring>

namespace SNE::Engine::Renderer::Vulkan {
    auto hasRequiredExtensions(
        const std::vector<const char *> &required_extensions,
        const std::vector<VkExtensionProperties> &available_extensions)
        -> bool {
        for (const char *required_extension : required_extensions) {
            bool found = false;
            for (const VkExtensionProperties &extension :
                 available_extensions) {
                if (std::strcmp(required_extension, extension.extensionName) ==
                    0) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                return false;
            }
        }
        return true;
    }
} // namespace SNE::Engine::Renderer::Vulkan
