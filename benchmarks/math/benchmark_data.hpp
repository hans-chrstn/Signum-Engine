#pragma once

#include <array>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstdint>
#include <random>
#include <type_traits>
#include <utility>

namespace SignumBench {
    inline constexpr std::size_t kSampleCount = 64U;
    inline constexpr std::uint32_t kRandomSeed = 42U;

    template <typename Value>
    using Component = decltype(std::declval<Value>().x);

    template <typename Scalar>
    [[nodiscard]] auto makeDistribution(Scalar minimum, Scalar maximum)
        -> std::uniform_real_distribution<Scalar> {
        return std::uniform_real_distribution<Scalar>{minimum, maximum};
    }

    template <typename Value>
    [[nodiscard]] auto generateVector(std::mt19937 &generator,
                                      std::uniform_real_distribution<Component<Value>> &distribution)
        -> Value {
        Value vector{};
        vector.x = distribution(generator);
        vector.y = distribution(generator);
        if constexpr (requires { vector.z; }) {
            vector.z = distribution(generator);
        }
        if constexpr (requires { vector.w; }) {
            vector.w = distribution(generator);
        }
        return vector;
    }

    template <typename Value>
    struct Pair {
        Value first{};
        Value second{};
    };

    template <typename Dataset, typename Operation>
    void run(benchmark::State &state, Dataset &dataset, Operation operation) {
        std::size_t index{};
        for ([[maybe_unused]] auto iteration : state) {
            auto &sample = dataset[index];
            benchmark::DoNotOptimize(sample);
            auto result = operation(sample);
            benchmark::DoNotOptimize(result);
            ++index;
            if (index == kSampleCount) {
                index = 0U;
            }
        }
    }
} // namespace SignumBench
