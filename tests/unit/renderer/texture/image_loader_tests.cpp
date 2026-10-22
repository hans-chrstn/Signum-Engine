#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/texture/image_loader.hpp"
#include <array>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Error = SNE::Engine::Core::Error;

namespace {
    class ImageLoaderTests : public testing::Test {
      protected:
        std::filesystem::path m_Path;

        auto SetUp() -> void override {
            const auto timestamp =
                std::chrono::steady_clock::now().time_since_epoch().count();

            m_Path =
                std::filesystem::temp_directory_path() /
                (std::string{"signum_image_loader_"} +
                 testing::UnitTest::GetInstance()->current_test_info()->name() +
                 "_" + std::to_string(timestamp) + ".ppm");
        }

        auto TearDown() -> void override {
            std::error_code error;
            std::filesystem::remove(m_Path, error);
        }

        auto writeImageBytes(std::string_view bytes) -> void {
            std::ofstream file(m_Path, std::ios::binary);

            ASSERT_TRUE(file.is_open());

            file.write(bytes.data(),
                       static_cast<std::streamsize>(bytes.size()));

            ASSERT_TRUE(file.good());
        }
    };
} // namespace

TEST_F(ImageLoaderTests, DecodesRgbImageIntoRgba8) {
    constexpr auto kPpmImage = std::to_array("P6\n2 1\n255\n"
                                             "\xFF\x00\x00"
                                             "\x00\x80\xFF");

    writeImageBytes(std::string_view{
        kPpmImage.data(),
        kPpmImage.size() - 1U,
    });

    const Renderer::Rgba8Image image = Renderer::loadRgba8Image(m_Path);

    EXPECT_EQ(image.width, 2U);
    EXPECT_EQ(image.height, 1U);

    const std::vector<std::byte> expected{
        std::byte{255}, std::byte{0},   std::byte{0},   std::byte{255},
        std::byte{0},   std::byte{128}, std::byte{255}, std::byte{255},
    };

    EXPECT_EQ(image.pixels, expected);
}

TEST_F(ImageLoaderTests, MissingFileThrowsImageLoadFailed) {
    ASSERT_FALSE(std::filesystem::exists(m_Path));

    try {
        static_cast<void>(Renderer::loadRgba8Image(m_Path));
        FAIL() << "Expected EngineError for missing image";
    } catch (const Error::EngineError &error) {
        EXPECT_EQ(error.getCode(), Error::Code::ImageLoadFailed);
    }
}

TEST_F(ImageLoaderTests, InvalidImageThrowsImageLoadFailed) {
    writeImageBytes("This is not a valid image.");

    try {
        static_cast<void>(Renderer::loadRgba8Image(m_Path));
        FAIL() << "Expected EngineError for invalid image";
    } catch (const Error::EngineError &error) {
        EXPECT_EQ(error.getCode(), Error::Code::ImageLoadFailed);
    }
}
