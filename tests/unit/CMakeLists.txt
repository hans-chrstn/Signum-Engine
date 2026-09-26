add_executable(
  signum_unit_tests
  core/error/engine_error_tests.cpp
  core/error/error_code_tests.cpp
  core/error/native_error_tests.cpp
  core/error/diagnostic_formatter_tests.cpp
  core/error/diagnostic_writer_tests.cpp
  core/error/subsystem_tests.cpp
  renderer/vulkan/vulkan_result_tests.cpp
  renderer/vulkan/vulkan_extension_support_tests.cpp
  renderer/vulkan/vulkan_queue_families_tests.cpp
  renderer/vulkan/vulkan_device_selection_tests.cpp
)

target_link_libraries(
  signum_unit_tests
  PRIVATE
    signum_engine
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(
  signum_unit_tests
  TEST_PREFIX "unit::"
  PROPERTIES
    LABELS "unit"
)
