#include "engine/core/error/native_error.hpp"
#include <gtest/gtest.h>

namespace Error = SNE::Engine::Core::Error;
constexpr int kTestNativeCode = 123;

TEST(NativeErrorTests, PreservesNativeErrorCode) {
    Error::NativeError native_error =
        Error::NativeError(kTestNativeCode, "Test");

    EXPECT_EQ(native_error.getNativeCode(), kTestNativeCode);
}

TEST(NativeErrorTests, PreservesNativeErrorDescription) {
    Error::NativeError native_error =
        Error::NativeError(kTestNativeCode, "Test");

    EXPECT_EQ(native_error.getNativeDescription(), "Test");
}
