find_package(benchmark CONFIG REQUIRED)

add_executable(
  signum_math_benchmarks
  math/matrix_benchmarks.cpp
)

target_link_libraries(
  signum_math_benchmarks
  PRIVATE
    signum_engine
    benchmark::benchmark_main
)
