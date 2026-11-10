#include "subsystem.hpp"
#include "engine/core/config/compiler_features.hpp"
#include "error_metadata.hpp"
#include <exception>

#if SNE_HAS_CPP26_REFLECTION
    #include <meta>
#endif

namespace SNE::Engine::Core::Error {
    auto getSubsystemFor(Code code) noexcept -> Subsystem {
        const ErrorMetadata *metadata = findErrorMetadata(code);
        if (metadata != nullptr) {
            return metadata->subsystem;
        }

        std::terminate();
    }

    auto toString(Subsystem subsystem) noexcept -> std::string_view {

#if SNE_HAS_CPP26_REFLECTION
        template for (constexpr auto enumerator : std::define_static_array(
                          std::meta::enumerators_of(^^Subsystem))) {
            static_assert(!std::meta::identifier_of(enumerator).empty());
            if (subsystem == [:enumerator:]) {
                return std::meta::identifier_of(enumerator);
            }
        }

#else
        switch (subsystem) {
        case Subsystem::Platform:
            return "Platform";
        case Subsystem::Core:
            return "Core";
        case Subsystem::Editor:
            return "Editor";
        case Subsystem::Renderer:
            return "Renderer";
        case Subsystem::Vulkan:
            return "Vulkan";
        }
#endif

        return "Unknown";
    }
} // namespace SNE::Engine::Core::Error
