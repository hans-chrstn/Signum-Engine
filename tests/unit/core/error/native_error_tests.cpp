#include "engine/core/error/native_error.hpp"
#include <catch2/catch_test_macros.hpp>

namespace Error = SNE::Engine::Core::Error;

TEST_CASE("NativeError preserves native error code") {
    Error::NativeError native_error = Error::NativeError(123, "Test");

    REQUIRE(native_error.getNativeCode() == 123);
}

TEST_CASE("NativeError preserves native error description") {
    Error::NativeError native_error = Error::NativeError(123, "Test");

    REQUIRE(native_error.getNativeDescription() == "Test");
}
