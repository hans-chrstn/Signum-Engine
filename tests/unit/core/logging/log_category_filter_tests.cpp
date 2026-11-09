#include "engine/core/logging/log_category_filter.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace Logging = SNE::Engine::Core::Logging;

TEST(LogCategoryFilterTests, EmptyAllowlistAcceptsAllCategories) {
    const std::vector<std::string> categories;

    EXPECT_TRUE(Logging::passesCategoryFilter("Renderer", categories));
    EXPECT_TRUE(Logging::passesCategoryFilter("Vulkan", categories));
    EXPECT_TRUE(Logging::passesCategoryFilter("", categories));
}

TEST(LogCategoryFilterTests, AcceptsRegisteredCategories) {
    const std::vector<std::string> categories{"Renderer", "Vulkan"};

    EXPECT_TRUE(Logging::passesCategoryFilter("Renderer", categories));
    EXPECT_TRUE(Logging::passesCategoryFilter("Vulkan", categories));
}

TEST(LogCategoryFilterTests, RejectsUnregisteredCategories) {
    const std::vector<std::string> categories{"Renderer", "Vulkan"};

    EXPECT_FALSE(Logging::passesCategoryFilter("Editor", categories));
    EXPECT_FALSE(Logging::passesCategoryFilter("Core", categories));
    EXPECT_FALSE(Logging::passesCategoryFilter("", categories));
}

TEST(LogCategoryFilterTests, MatchingIsCaseSensitive) {
    const std::vector<std::string> categories{"Renderer"};

    EXPECT_TRUE(Logging::passesCategoryFilter("Renderer", categories));
    EXPECT_FALSE(Logging::passesCategoryFilter("renderer", categories));
    EXPECT_FALSE(Logging::passesCategoryFilter("RENDERER", categories));
}

TEST(LogCategoryFilterTests, RequiresExactCategoryMatch) {
    const std::vector<std::string> categories{"Renderer"};

    EXPECT_FALSE(Logging::passesCategoryFilter("Render", categories));
    EXPECT_FALSE(Logging::passesCategoryFilter("Renderer.Shader", categories));
    EXPECT_FALSE(Logging::passesCategoryFilter("Renderer ", categories));
}

TEST(LogCategoryFilterTests, AcceptsExplicitlyRegisteredEmptyCategory) {
    const std::vector<std::string> categories{""};

    EXPECT_TRUE(Logging::passesCategoryFilter("", categories));
    EXPECT_FALSE(Logging::passesCategoryFilter("Renderer", categories));
}
