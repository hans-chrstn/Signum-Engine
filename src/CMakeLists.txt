add_library(
  signum_engine
  STATIC
    engine/core/application.cpp
    engine/core/error/engine_error.cpp
    engine/core/error/native_error.cpp
    engine/platform/glfw_context.cpp
    engine/platform/window.cpp
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
