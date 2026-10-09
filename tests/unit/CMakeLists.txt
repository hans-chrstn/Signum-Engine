add_executable(
  signum_unit_tests
  core/error/engine_error_tests.cpp
  core/error/error_code_tests.cpp
  core/error/native_error_tests.cpp
  core/error/diagnostic_formatter_tests.cpp
  core/error/diagnostic_writer_tests.cpp
  core/error/subsystem_tests.cpp
  core/assert/assertion_type_tests.cpp
  core/assert/assertion_failure_tests.cpp
  core/assert/assertion_formatter_tests.cpp
  core/assert/assertion_writer_tests.cpp
  core/assert/assertion_handler_tests.cpp
  core/numeric/checked_conversion_tests.cpp
  core/math/matrix4_tests.cpp
  core/math/vector2_tests.cpp
  core/math/vector3_tests.cpp
  core/math/vector4_tests.cpp
  renderer/vulkan/common/vulkan_result_tests.cpp
  renderer/vulkan/common/vulkan_extension_support_tests.cpp
  renderer/vulkan/device/vulkan_queue_families_tests.cpp
  renderer/vulkan/device/vulkan_device_selection_tests.cpp
  renderer/vulkan/common/vulkan_api_version_tests.cpp
  renderer/vulkan/device/vulkan_queue_requests_tests.cpp
  renderer/vulkan/presentation/vulkan_swapchain_selection_tests.cpp
  renderer/vulkan/presentation/vulkan_swapchain_result_policy_tests.cpp
  renderer/vulkan/device/vulkan_device_tests.cpp
  renderer/vulkan/device/vulkan_device_features_tests.cpp
  renderer/vulkan/memory/vulkan_memory_policy_tests.cpp
  renderer/vulkan/device/vulkan_device_extensions_tests.cpp
  renderer/vulkan/memory/vulkan_byte_range_tests.cpp
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
