add_executable(
  signum_integration_tests
  platform/window_keyboard_integration_tests.cpp
  platform/window_cursor_capture_integration_tests.cpp
  renderer/vulkan/vulkan_integration_test_fixture.cpp
  renderer/vulkan/vulkan_renderer_mesh_replacement_integration_tests.cpp
  renderer/vulkan/vulkan_renderer_texture_replacement_integration_tests.cpp
  renderer/vulkan/vulkan_renderer_material_integration_tests.cpp
  renderer/vulkan/vulkan_renderer_material_instance_integration_tests.cpp
  renderer/vulkan/vulkan_renderer_material_update_integration_tests.cpp
  renderer/vulkan/command/vulkan_immediate_submission_integration_tests.cpp
  renderer/vulkan/command/vulkan_transfer_commands_integration_tests.cpp
  renderer/vulkan/compute/vulkan_matrix_transform_integration_tests.cpp
  renderer/vulkan/descriptor/vulkan_descriptor_set_layout_integration_tests.cpp
  renderer/vulkan/descriptor/vulkan_descriptor_pool_integration_tests.cpp
  renderer/vulkan/descriptor/vulkan_descriptor_updates_integration_tests.cpp
  renderer/vulkan/descriptor/vulkan_layout_creation_integration_tests.cpp
  renderer/vulkan/device/vulkan_device_capabilities_integration_tests.cpp
  renderer/vulkan/device/vulkan_depth_format_integration_tests.cpp
  renderer/vulkan/memory/vulkan_buffer_integration_tests.cpp
  renderer/vulkan/memory/vulkan_memory_allocator_integration_tests.cpp
  renderer/vulkan/memory/vulkan_transfer_integration_tests.cpp
  renderer/vulkan/memory/vulkan_image_integration_tests.cpp
  renderer/vulkan/memory/vulkan_image_view_integration_tests.cpp
  renderer/vulkan/memory/vulkan_sampler_integration_tests.cpp
  renderer/vulkan/pipeline/vulkan_push_constant_reflection_integration_tests.cpp
  renderer/vulkan/pipeline/vulkan_graphics_pipeline_integration_tests.cpp
  renderer/vulkan/pipeline/vulkan_shader_module_integration_tests.cpp
  renderer/vulkan/pipeline/vulkan_graphics_pipeline_cache_integration_tests.cpp
  renderer/vulkan/presentation/vulkan_swapchain_retirement_integration_tests.cpp
)

target_link_libraries(
  signum_integration_tests
  PRIVATE
    signum_engine
    GTest::gtest_main
)

if(SIGNUM_ENABLE_RUNTIME_SHADER_COMPILATION)
  target_sources(
    signum_integration_tests
    PRIVATE
      renderer/shader/shader_source_compiler_integration_tests.cpp
  )

  target_link_libraries(
    signum_integration_tests
    PRIVATE
      signum_shader_compiler
  )
endif()

target_compile_definitions(
  signum_integration_tests
  PRIVATE
    SIGNUM_TEST_SHADER_DIR="${CMAKE_SOURCE_DIR}/build/shaders"
    SIGNUM_TEST_FIXTURE_DIR="${CMAKE_SOURCE_DIR}/tests/fixtures/shaders"
)

include(GoogleTest)

gtest_discover_tests(
  signum_integration_tests
  TEST_PREFIX "integration::"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  PROPERTIES
    LABELS "integration"
    ENVIRONMENT "ASAN_OPTIONS=detect_leaks=0"
)
