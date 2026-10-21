#include "engine/renderer/vulkan/presentation/fail_closed_replacement.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
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

    enum class DestructionPart : std::uint8_t {
        Framebuffer,
        ImageView,
        Image,
    };

    struct DependencyDestructionState {
        static constexpr std::size_t kMaximumEvents{3U};

        std::array<DestructionPart, kMaximumEvents> events{};
        std::size_t event_count{};

        bool image_alive{};
        bool view_alive{};
        bool framebuffer_alive{};
        bool invalid_destruction_order{};

        auto record(DestructionPart part) noexcept -> void {
            if (event_count >= events.size()) {
                invalid_destruction_order = true;
                return;
            }

            events[event_count] = part;
            ++event_count;
        }
    };

    class TrackedImageOwner {
      private:
        DependencyDestructionState *m_State{};

      public:
        explicit TrackedImageOwner(DependencyDestructionState &state) noexcept
            : m_State(&state) {
            state.image_alive = true;
        }

        ~TrackedImageOwner() noexcept {
            if (m_State == nullptr) {
                return;
            }

            if (m_State->view_alive || m_State->framebuffer_alive) {
                m_State->invalid_destruction_order = true;
            }

            m_State->image_alive = false;
            m_State->record(DestructionPart::Image);
        }

        TrackedImageOwner(const TrackedImageOwner &) = delete;

        auto operator=(const TrackedImageOwner &)
            -> TrackedImageOwner & = delete;

        TrackedImageOwner(TrackedImageOwner &&other) noexcept
            : m_State(std::exchange(other.m_State, nullptr)) {}
    };

    class TrackedImageView {
      private:
        DependencyDestructionState *m_State{};

      public:
        explicit TrackedImageView(DependencyDestructionState &state) noexcept
            : m_State(&state) {
            state.view_alive = true;
        }

        ~TrackedImageView() noexcept {
            if (m_State == nullptr) {
                return;
            }

            if (!m_State->image_alive || m_State->framebuffer_alive) {
                m_State->invalid_destruction_order = true;
            }

            m_State->view_alive = false;
            m_State->record(DestructionPart::ImageView);
        }

        TrackedImageView(const TrackedImageView &) = delete;

        auto operator=(const TrackedImageView &) -> TrackedImageView & = delete;

        TrackedImageView(TrackedImageView &&other) noexcept
            : m_State(std::exchange(other.m_State, nullptr)) {}
    };

    class TrackedFramebuffer {
      private:
        DependencyDestructionState *m_State{};

      public:
        explicit TrackedFramebuffer(DependencyDestructionState &state,
                                    bool fail_construction = false)
            : m_State(&state) {
            if (fail_construction) {
                throw std::runtime_error{"Framebuffer creation failed"};
            }

            state.framebuffer_alive = true;
        }

        ~TrackedFramebuffer() noexcept {
            if (m_State == nullptr) {
                return;
            }

            if (!m_State->image_alive || !m_State->view_alive) {
                m_State->invalid_destruction_order = true;
            }

            m_State->framebuffer_alive = false;
            m_State->record(DestructionPart::Framebuffer);
        }

        TrackedFramebuffer(const TrackedFramebuffer &) = delete;

        auto operator=(const TrackedFramebuffer &)
            -> TrackedFramebuffer & = delete;

        TrackedFramebuffer(TrackedFramebuffer &&other) noexcept
            : m_State(std::exchange(other.m_State, nullptr)) {}
    };

    struct DependencyTrackedResourceGroup {
        TrackedImageOwner image;
        TrackedImageView view;
        TrackedFramebuffer framebuffer;

        explicit DependencyTrackedResourceGroup(
            DependencyDestructionState &state,
            bool fail_framebuffer_construction = false)
            : image(state), view(state),
              framebuffer(state, fail_framebuffer_construction) {}
    };

    static_assert(
        std::is_nothrow_move_constructible_v<DependencyTrackedResourceGroup>);
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

TEST(FailClosedReplacementTests,
     FailedReplacementDestroysResourceGroupsInDependencyOrder) {
    constexpr std::size_t kExpectedDestructionCount{3U};

    DependencyDestructionState previous_state{};
    DependencyDestructionState replacement_state{};

    std::optional<DependencyTrackedResourceGroup> current{
        std::in_place,
        previous_state,
    };

    EXPECT_THROW(
        Vulkan::replaceRetiringResource(
            current,
            [&]() -> DependencyTrackedResourceGroup {
                [[maybe_unused]] DependencyTrackedResourceGroup replacement{
                    replacement_state,
                };

                throw std::runtime_error{"Replacement creation interrupted"};
            }),
        std::runtime_error);

    const std::array expected_destruction_order{
        DestructionPart::Framebuffer,
        DestructionPart::ImageView,
        DestructionPart::Image,
    };

    EXPECT_FALSE(current.has_value());

    EXPECT_EQ(previous_state.event_count, kExpectedDestructionCount);
    EXPECT_EQ(replacement_state.event_count, kExpectedDestructionCount);

    EXPECT_EQ(previous_state.events, expected_destruction_order);
    EXPECT_EQ(replacement_state.events, expected_destruction_order);

    EXPECT_FALSE(previous_state.invalid_destruction_order);
    EXPECT_FALSE(replacement_state.invalid_destruction_order);

    EXPECT_FALSE(previous_state.image_alive);
    EXPECT_FALSE(previous_state.view_alive);
    EXPECT_FALSE(previous_state.framebuffer_alive);

    EXPECT_FALSE(replacement_state.image_alive);
    EXPECT_FALSE(replacement_state.view_alive);
    EXPECT_FALSE(replacement_state.framebuffer_alive);
}

TEST(FailClosedReplacementTests,
     PartialReplacementConstructionPreservesDependencyDestructionOrder) {
    constexpr std::size_t kPreviousDestructionCount{3U};
    constexpr std::size_t kPartialDestructionCount{2U};

    DependencyDestructionState previous_state{};
    DependencyDestructionState replacement_state{};

    std::optional<DependencyTrackedResourceGroup> current{
        std::in_place,
        previous_state,
    };

    EXPECT_THROW(Vulkan::replaceRetiringResource(
                     current,
                     [&]() -> DependencyTrackedResourceGroup {
                         return DependencyTrackedResourceGroup{
                             replacement_state,
                             true,
                         };
                     }),
                 std::runtime_error);

    const std::array expected_previous_order{
        DestructionPart::Framebuffer,
        DestructionPart::ImageView,
        DestructionPart::Image,
    };

    EXPECT_FALSE(current.has_value());

    EXPECT_EQ(previous_state.event_count, kPreviousDestructionCount);
    EXPECT_EQ(previous_state.events, expected_previous_order);

    ASSERT_EQ(replacement_state.event_count, kPartialDestructionCount);

    EXPECT_EQ(replacement_state.events[0], DestructionPart::ImageView);
    EXPECT_EQ(replacement_state.events[1], DestructionPart::Image);

    EXPECT_FALSE(previous_state.invalid_destruction_order);
    EXPECT_FALSE(replacement_state.invalid_destruction_order);

    EXPECT_FALSE(previous_state.image_alive);
    EXPECT_FALSE(previous_state.view_alive);
    EXPECT_FALSE(previous_state.framebuffer_alive);

    EXPECT_FALSE(replacement_state.image_alive);
    EXPECT_FALSE(replacement_state.view_alive);
    EXPECT_FALSE(replacement_state.framebuffer_alive);
}
