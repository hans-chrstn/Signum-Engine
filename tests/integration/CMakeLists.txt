add_executable(
  signum_integration_tests
  renderer/vulkan/vulkan_integration_test_fixture.cpp
  renderer/vulkan/device/vulkan_device_capabilities_integration_tests.cpp
  renderer/vulkan/memory/vulkan_buffer_integration_tests.cpp
  renderer/vulkan/memory/vulkan_memory_allocator_integration_tests.cpp
  renderer/vulkan/memory/vulkan_transfer_integration_tests.cpp
  renderer/vulkan/memory/vulkan_image_integration_tests.cpp
  renderer/vulkan/command/vulkan_immediate_submission_integration_tests.cpp
  renderer/vulkan/command/vulkan_transfer_commands_integration_tests.cpp
)

target_link_libraries(
  signum_integration_tests
  PRIVATE
    signum_engine
    GTest::gtest_main
)

include(GoogleTest)

gtest_discover_tests(
  signum_integration_tests
  TEST_PREFIX "integration::"
  PROPERTIES
    LABELS "integration"
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
)
