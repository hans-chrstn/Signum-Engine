add_executable(
  signum_integration_tests
  renderer/vulkan/vulkan_device_capabilities_integration_tests.cpp
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
