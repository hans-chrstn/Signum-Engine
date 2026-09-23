#include "engine/core/error/native_error.hpp"
#include <catch2/catch_test_macros.hpp>

TEST_CASE("NativeError preserves native error code") {
    SNE::Engine::Core::Error::NativeError native_error =
        SNE::Engine::Core::Error::NativeError(123, "Test");

    REQUIRE(native_error.getNativeCode() == 123);
}

TEST_CASE("NativeError preserves native error description") {
    SNE::Engine::Core::Error::NativeError native_error =
        SNE::Engine::Core::Error::NativeError(123, "Test");

    REQUIRE(native_error.getNativeDescription() == "Test");
}
