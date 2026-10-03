#include <cstdint>
#include <filesystem>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Loads compiled SPIR-V bytecode from a binary file.
     *
     * Reads the supplied file as a sequence of 32-bit SPIR-V words suitable for
     * Vulkan shader-module creation.
     *
     * @param path Path to the compiled SPIR-V file.
     * @return SPIR-V bytecode stored as 32-bit words.
     *
     * @throws Core::Error::EngineError if the file cannot be opened, has an
     * invalid byte size, or cannot be read completely.
     */
    [[nodiscard]] auto loadSpirv(const std::filesystem::path &path)
        -> std::vector<std::uint32_t>;
} // namespace SNE::Engine::Renderer::Vulkan
