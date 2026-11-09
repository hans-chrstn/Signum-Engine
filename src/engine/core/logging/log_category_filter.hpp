#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Determines whether a category is permitted by an allowlist.
     *
     * An empty allowlist permits all categories.
     *
     * When the allowlist is not empty, the supplied category must
     * exactly match one of its entries. Matching is case-sensitive.
     *
     * This function does not allocate, modify the allowlist,
     * or produce diagnostic output.
     *
     * @param category Category of the incoming logging event.
     * @param allowed_categories Permitted category names.
     * @return True if the category is accepted; otherwise false.
     */
    [[nodiscard]] auto
    passesCategoryFilter(std::string_view category,
                         const std::vector<std::string> &allowed_categories)
        -> bool;
} // namespace SNE::Engine::Core::Logging
