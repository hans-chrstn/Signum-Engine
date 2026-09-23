#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include <catch2/catch_test_macros.hpp>

namespace Error = SNE::Engine::Core::Error;

TEST_CASE("GLFW initialization errors belong to Platform") {
    REQUIRE(Error::getSubsystemFor(Error::Code::GlfwInitializationFailed) ==
            Error::Subsystem::Platform);
}

TEST_CASE("Window creation errors belong to Platform") {
    REQUIRE(Error::getSubsystemFor(Error::Code::WindowCreationFailed) ==
            Error::Subsystem::Platform);
}

TEST_CASE("Subsystem names are readable") {
    REQUIRE(Error::toString(Error::Subsystem::Platform) == "Platform");
    REQUIRE(Error::toString(Error::Subsystem::Core) == "Core");
    REQUIRE(Error::toString(Error::Subsystem::Editor) == "Editor");
    REQUIRE(Error::toString(Error::Subsystem::Renderer) == "Renderer");
    REQUIRE(Error::toString(Error::Subsystem::Vulkan) == "Vulkan");
}

TEST_CASE("Unknown subsystems have a fallback name") {
    REQUIRE(Error::toString(static_cast<Error::Subsystem>(-1)) == "Unknown");
}
