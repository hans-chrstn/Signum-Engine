set(
  IMGUI_SOURCE_DIR
  "$ENV{IMGUI_SOURCE_DIR}"
  CACHE PATH "Path to Dear ImGui source"
)

if(NOT EXISTS "${IMGUI_SOURCE_DIR}/imgui.cpp")
  message(
    FATAL_ERROR
    "Dear ImGui source directory is unavailable"
  )
endif()

add_library(
  signum_editor_imgui
  STATIC
    "${IMGUI_SOURCE_DIR}/imgui.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_draw.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_tables.cpp"
    "${IMGUI_SOURCE_DIR}/imgui_widgets.cpp"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
)

target_include_directories(
  signum_editor_imgui
  SYSTEM PUBLIC
    "${IMGUI_SOURCE_DIR}"
    "${IMGUI_SOURCE_DIR}/backends"
)

target_link_libraries(
  signum_editor_imgui
  PUBLIC
    Vulkan::Vulkan
    glfw
)

add_executable(
  signum_editor
  main.cpp
  editor_workspace.cpp
  imgui/editor_imgui_context.cpp
  imgui/editor_imgui_vulkan_backend.cpp
  panels/editor_status_panel.cpp
  panels/about_window.cpp
)

target_link_libraries(
  signum_editor
  PRIVATE
    signum_engine
    signum_editor_imgui
)
