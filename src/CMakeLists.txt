add_library(
  signum_engine
  STATIC
    engine/core/application.cpp
    engine/core/error/engine_error.cpp
    engine/core/error/native_error.cpp
    engine/core/error/error_code.cpp
    engine/core/error/subsystem.cpp
    engine/core/error/diagnostic_formatter.cpp
    engine/core/error/diagnostic_writer.cpp
    engine/core/assert/assertion_type.cpp
    engine/core/assert/assertion_failure.cpp
    engine/core/assert/assertion_formatter.cpp
    engine/core/assert/assertion_writer.cpp
    engine/core/assert/assertion_handler.cpp
    engine/platform/error/glfw_error.cpp
    engine/platform/glfw_context.cpp
    engine/platform/window.cpp
    engine/renderer/vulkan/vulkan_extension_support.cpp
    engine/renderer/vulkan/vulkan_instance.cpp
    engine/renderer/vulkan/vulkan_surface.cpp
    engine/renderer/vulkan/vulkan_result.cpp
    engine/renderer/vulkan/vulkan_device_discovery.cpp
    engine/renderer/vulkan/vulkan_device_capabilities.cpp
    engine/renderer/vulkan/vulkan_queue_families.cpp
    engine/renderer/vulkan/vulkan_device_extensions.cpp
    engine/renderer/vulkan/vulkan_device_selection.cpp
    engine/renderer/vulkan/vulkan_device_features.cpp
    engine/renderer/vulkan/vulkan_queue_requests.cpp
    engine/renderer/vulkan/vulkan_device.cpp
    engine/renderer/vulkan/vulkan_swapchain_support.cpp
    engine/renderer/vulkan/vulkan_swapchain_selection.cpp
    engine/renderer/vulkan/vulkan_swapchain.cpp
    engine/renderer/vulkan/vulkan_command_pool.cpp
    engine/renderer/vulkan/vulkan_frame_resources.cpp
    engine/renderer/vulkan/vulkan_renderer.cpp
    engine/renderer/vulkan/vulkan_semaphore.cpp
    engine/renderer/vulkan/vulkan_fence.cpp
    engine/renderer/vulkan/vulkan_spirv.cpp
    engine/renderer/vulkan/vulkan_shader_module.cpp
    engine/renderer/vulkan/vulkan_graphics_pipeline.cpp
    engine/renderer/vulkan/vulkan_memory_allocator.cpp
    engine/renderer/vulkan/vulkan_buffer.cpp
    engine/renderer/vulkan/vulkan_memory_policy.cpp
    engine/renderer/vulkan/vulkan_image.cpp
)

target_include_directories(
  signum_engine
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
  signum_engine
  PUBLIC
    signum_project_options
  PRIVATE
    Vulkan::Vulkan
    glfw
    GPUOpen::VulkanMemoryAllocator
)

add_executable(
  signum
  main.cpp
)

target_link_libraries(
  signum
  PRIVATE
    signum_engine
)

set_target_properties(
  signum
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

install(
  TARGETS signum
  RUNTIME DESTINATION bin
)
