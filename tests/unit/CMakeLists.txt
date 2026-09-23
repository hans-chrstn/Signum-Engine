add_executable(
  signum_unit_tests
  core/error/engine_error_tests.cpp
  core/error/native_error_tests.cpp
  core/error/subsystem_tests.cpp
)

target_link_libraries(
  signum_unit_tests
  PRIVATE
    signum_engine
    Catch2::Catch2WithMain
)

include(Catch)

catch_discover_tests(
  signum_unit_tests
  TEST_PREFIX "unit::"
  PROPERTIES
    LABELS "unit"
)
