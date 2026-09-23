#include "engine/core/error/native_error.hpp"
#include <catch2/catch_test_macros.hpp>

namespace Error = SNE::Engine::Core::Error;
constexpr int kTestNativeCode = 123;

TEST_CASE("NativeError preserves native error code") {
    Error::NativeError native_error =
        Error::NativeError(kTestNativeCode, "Test");

    REQUIRE(native_error.getNativeCode() == kTestNativeCode);
}

TEST_CASE("NativeError preserves native error description") {
    Error::NativeError native_error =
        Error::NativeError(kTestNativeCode, "Test");

    REQUIRE(native_error.getNativeDescription() == "Test");
}
