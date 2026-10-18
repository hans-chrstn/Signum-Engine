#include "material_resource_binding_compatibility.hpp"
#include <cstddef>

namespace SNE::Engine::Renderer {
    auto isMaterialResourceBindingCompatible(
        const MaterialResourceBinding &provided,
        const Shader::ShaderDescriptorBinding &required) noexcept -> bool {
        return (provided.set == required.set) &&
               (provided.binding == required.binding) &&
               (provided.type == required.type) &&
               (provided.array_element < required.count);
    }

    auto areMaterialResourceBindingsCompatible(
        std::span<const MaterialResourceBinding> provided,
        std::span<const Shader::ShaderDescriptorBinding> required) noexcept
        -> bool {
        for (std::size_t i{}; i < provided.size(); ++i) {
            for (std::size_t j{i + 1U}; j < provided.size(); ++j) {
                if (provided[i].array_element == provided[j].array_element &&
                    provided[i].binding == provided[j].binding &&
                    provided[i].set == provided[j].set) {
                    return false;
                }
            }
        }

        for (const auto &requirement : required) {
            if (requirement.count == 0) {
                return false;
            }

            for (std::uint32_t i{}; i < requirement.count; ++i) {
                bool found = false;

                for (const auto &binding : provided) {
                    if (isMaterialResourceBindingCompatible(binding,
                                                            requirement) &&
                        binding.array_element == i) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }
        }

        return true;
    }
} // namespace SNE::Engine::Renderer
