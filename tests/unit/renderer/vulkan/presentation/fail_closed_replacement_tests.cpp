#include "engine/renderer/vulkan/presentation/fail_closed_replacement.hpp"
#include <exception>
#include <gtest/gtest.h>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    struct DestructionCounts {
        int previous{};
        int replacement{};
    };

    enum class ResourceVersion : std::uint8_t {
        Previous,
        Replacement,
    };

    class TrackedResource {
      private:
        DestructionCounts *m_Counts{};
        ResourceVersion m_Version{};

      public:
        TrackedResource(DestructionCounts &counts,
                        ResourceVersion version) noexcept
            : m_Counts(&counts), m_Version(version) {}

        ~TrackedResource() noexcept {
            if (m_Counts == nullptr) {
                return;
            }

            if (m_Version == ResourceVersion::Previous) {
                ++m_Counts->previous;
            } else {
                ++m_Counts->replacement;
            }
        }

        TrackedResource(const TrackedResource &) = delete;

        auto operator=(const TrackedResource &) -> TrackedResource & = delete;

        TrackedResource(TrackedResource &&other) noexcept
            : m_Counts(std::exchange(other.m_Counts, nullptr)),
              m_Version(other.m_Version) {}

        auto operator=(TrackedResource &&) -> TrackedResource & = delete;
    };

    static_assert(std::is_nothrow_move_constructible_v<TrackedResource>);
} // namespace

TEST(FailClosedReplacementTests,
     SuccessfulReplacementDestroysPreviousResource) {
    DestructionCounts counts{};

    std::optional<TrackedResource> current{std::in_place, counts,
                                           ResourceVersion::Previous};

    Vulkan::replaceRetiringResource(current, [&] -> TrackedResource {
        return TrackedResource{counts, ResourceVersion::Replacement};
    });

    EXPECT_TRUE(current.has_value());
    EXPECT_EQ(counts.previous, 1);
    EXPECT_EQ(counts.replacement, 0);

    current.reset();

    EXPECT_EQ(counts.replacement, 1);
}

TEST(FailClosedReplacementTests, BuilderFailureClearsPreviousResource) {
    DestructionCounts counts{};

    std::optional<TrackedResource> current{std::in_place, counts,
                                           ResourceVersion::Previous};

    EXPECT_THROW(Vulkan::replaceRetiringResource(current,
                                                 [] -> TrackedResource {
                                                     throw std::runtime_error{
                                                         "Replacement failed"};
                                                 }),
                 std::runtime_error);

    EXPECT_FALSE(current.has_value());
    EXPECT_EQ(counts.previous, 1);
}

TEST(FailClosedReplacementTests,
     PartialConstructionFailureDestroysTemporaryResources) {
    DestructionCounts counts{};

    std::optional<TrackedResource> current{std::in_place, counts,
                                           ResourceVersion::Previous};

    EXPECT_THROW(Vulkan::replaceRetiringResource(
                     current,
                     [&] -> TrackedResource {
                         [[maybe_unused]] TrackedResource temporary{
                             counts, ResourceVersion::Replacement};

                         throw std::runtime_error{"Construction interrupted"};
                     }),
                 std::runtime_error);

    EXPECT_FALSE(current.has_value());
    EXPECT_EQ(counts.previous, 1);
    EXPECT_EQ(counts.replacement, 1);
}

TEST(FailClosedReplacementTests,
     InitialConstructionWorksWithoutPreviousResource) {
    DestructionCounts counts{};
    std::optional<TrackedResource> current;

    Vulkan::replaceRetiringResource(current, [&] -> TrackedResource {
        return TrackedResource{counts, ResourceVersion::Replacement};
    });

    EXPECT_TRUE(current.has_value());
    EXPECT_EQ(counts.previous, 0);
    EXPECT_EQ(counts.replacement, 0);

    current.reset();

    EXPECT_EQ(counts.replacement, 1);
}

TEST(FailClosedReplacementTests, FailurePreservesOriginalException) {
    DestructionCounts counts{};

    std::optional<TrackedResource> current{std::in_place, counts,
                                           ResourceVersion::Previous};

    const std::exception_ptr original_exception =
        std::make_exception_ptr(std::runtime_error{"Original failure"});

    try {
        Vulkan::replaceRetiringResource(current, [&] -> TrackedResource {
            std::rethrow_exception(original_exception);
        });

        FAIL() << "Expected replacement to throw";
    } catch (...) {
        EXPECT_EQ(std::current_exception(), original_exception);
    }

    EXPECT_FALSE(current.has_value());
    EXPECT_EQ(counts.previous, 1);
}
