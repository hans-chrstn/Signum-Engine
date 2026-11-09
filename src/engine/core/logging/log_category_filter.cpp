#include "log_category_filter.hpp"
#include <algorithm>

namespace SNE::Engine::Core::Logging {
    auto
    passesCategoryFilter(std::string_view category,
                         const std::vector<std::string> &allowed_categories)
        -> bool {
        if (allowed_categories.empty()) {
            return true;
        }

        const auto result = std::ranges::find(allowed_categories, category);

        return result != allowed_categories.end();
    }
} // namespace SNE::Engine::Core::Logging
