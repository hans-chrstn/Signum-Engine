#include "gpu_resource_registry_id.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <atomic>
#include <limits>

namespace SNE::Engine::Renderer {
    auto allocateGpuResourceRegistryId() -> std::uint64_t {
        static std::atomic<std::uint64_t> count{1U};

        std::uint64_t expected = count.load(std::memory_order_relaxed);

        while (true) {
            if (expected == std::numeric_limits<std::uint64_t>::max()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Renderer,
                    "GPU resource registry identity space exhausted");
            }

            std::uint64_t desired = expected + 1U;

            bool changed = count.compare_exchange_weak(
                expected, desired, std::memory_order_relaxed);

            if (changed) {
                return expected;
            }
        }
    }
} // namespace SNE::Engine::Renderer
