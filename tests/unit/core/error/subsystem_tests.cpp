#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("GLFW initialization errors belong to Platform") {
    REQUIRE(SNE::Engine::Core::Error::getSubsystemFor(
                SNE::Engine::Core::Error::Code::GlfwInitializationFailed) ==
            SNE::Engine::Core::Error::Subsystem::Platform);
}

TEST_CASE("Window creation errors belong to Platform") {
    REQUIRE(SNE::Engine::Core::Error::getSubsystemFor(
                SNE::Engine::Core::Error::Code::WindowCreationFailed) ==
            SNE::Engine::Core::Error::Subsystem::Platform);
}
