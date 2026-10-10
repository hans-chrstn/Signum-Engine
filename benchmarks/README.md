# Mathematics Benchmarks

The mathematics benchmarks use Google Benchmark to measure vector, matrix, and quaternion operations. They are built separately from the engine's normal tests.

## Running

From the repository root:

```bash
just bench
```

To record five repetitions without overwriting the committed baseline:

```bash
./scripts/bench.sh \
    --benchmark_repetitions=5 \
    --benchmark_display_aggregates_only=true \
    --benchmark_out=build/bench/math-local.json \
    --benchmark_out_format=json
```

## Baseline

The initial results are in `results/baseline/math-full.json`.

| Setting                              | Recorded configuration                                        |
| ------------------------------------ | ------------------------------------------------------------- |
| Date                                 | 2026-10-10 15:36:50 UTC-04:00                                 |
| Benchmark source and baseline commit | `8f7000fbe2f37cdb5fcf7ad652aa6618a07b91e5`                    |
| Platform                             | NixOS, x86-64                                                 |
| CPU                                  | AMD Ryzen 9 7950X, 16 cores / 32 threads                      |
| Toolchain                            | GCC 16 development environment (`gcc16Stdenv` in `flake.nix`) |
| Compiler                             | GCC 16.2.0                                                    |
| Standard library                     | libstdc++ 16 (`__GLIBCXX__ = 20260807`)                       |
| Optimization flags                   | `-O3 -DNDEBUG`                                                |
| C++ standard                         | C++26                                                         |
| Build preset                         | `bench` (Release, tests and sanitizers disabled)              |
| Google Benchmark                     | 1.9.5                                                         |
| Measurements                         | 73 benchmarks, 5 repetitions each                             |
| CPU frequency scaling                | Enabled                                                       |

The exact compiler patch version, standard library version, and effective compiler flags were not saved in the baseline JSON. To inspect the current build, use `g++ --version` and `build/bench/compile_commands.json`. The Release configuration uses CMake's toolchain-dependent Release flags.

## Methodology

- Workloads use 64 pre-generated input samples and a fixed random seed of 42.
- Input generation and setup take place outside the measured loops.
- `benchmark::DoNotOptimize` is used to keep inputs and outputs observable to the compiler.
- Results are reported as time per benchmark iteration, including loop and sample-selection overhead.
- Matrix inversion uses invertible transformation matrices. Quaternion vector rotation is measured through conversion to a rotation matrix.

## Limitations

These are single-threaded microbenchmarks, not whole-engine performance measurements. CPU frequency scaling, system load, compiler options, and hardware can affect the results. Compare measurements made with the same workload and build configuration. Numerical correctness is covered by the math tests, not by the benchmarks.
