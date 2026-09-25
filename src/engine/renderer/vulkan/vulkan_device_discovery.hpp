#pragma once

#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    [[nodiscard]] auto enumeratePhysicalDevices(VkInstance instance)
        -> std::vector<VkPhysicalDevice>;
}
