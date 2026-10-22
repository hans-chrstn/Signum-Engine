#include "image_loader.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include <cstddef>
#include <cstring>
#include <limits>
#include <memory>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace SNE::Engine::Renderer {
    auto loadRgba8Image(const std::filesystem::path &path) -> Rgba8Image {
        int image_width{};
        int image_height{};
        int source_channels{};

        using DecodedPixels =
            std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>;

        DecodedPixels decoded_pixels{
            stbi_load(path.string().c_str(), &image_width, &image_height,
                      &source_channels, STBI_rgb_alpha),
            &stbi_image_free,
        };

        if (decoded_pixels == nullptr) {
            throw Core::Error::EngineError(
                Core::Error::Code::ImageLoadFailed,
                "Failed to load or decode image file", "Load RGBA8 Image");
        }

        if (image_width <= 0 || image_height <= 0) {
            throw Core::Error::EngineError(
                Core::Error::Code::ImageLoadFailed,
                "Decoded image dimensions must be greater than zero",
                "Validate RGBA8 Image Dimensions");
        }

        constexpr std::size_t kRgbaBytesPerPixel{
            static_cast<std::size_t>(STBI_rgb_alpha),
        };

        const auto width = static_cast<std::size_t>(image_width);
        const auto height = static_cast<std::size_t>(image_height);

        if (width >
                std::numeric_limits<std::size_t>::max() / kRgbaBytesPerPixel ||
            height > std::numeric_limits<std::size_t>::max() /
                         (width * kRgbaBytesPerPixel)) {

            throw Core::Error::EngineError(
                Core::Error::Code::ImageLoadFailed,
                "Decoded RGBA8 image exceeds the supported buffer size",
                "Validate RGBA8 Image Buffer Size");
        }

        const std::size_t pixel_byte_count =
            width * height * kRgbaBytesPerPixel;

        Rgba8Image image{};

        image.width = static_cast<std::uint32_t>(image_width);
        image.height = static_cast<std::uint32_t>(image_height);

        image.pixels.resize(pixel_byte_count);

        std::memcpy(image.pixels.data(), decoded_pixels.get(),
                    pixel_byte_count);

        return image;
    }
} // namespace SNE::Engine::Renderer
