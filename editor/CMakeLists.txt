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

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/Inter-Regular.ttf"
  "${CMAKE_CURRENT_BINARY_DIR}/assets/fonts/Inter-Regular.ttf"
  COPYONLY
)

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/LICENSE.txt"
  "${CMAKE_CURRENT_BINARY_DIR}/assets/fonts/LICENSE.txt"
  COPYONLY
)

add_executable(
  signum_editor
  main.cpp
  editor_workspace.cpp
  imgui/editor_imgui_context.cpp
  imgui/editor_imgui_vulkan_backend.cpp
  imgui/editor_imgui_style.cpp
  panels/editor_status_panel.cpp
  panels/about_window.cpp
  panels/hierarchy_panel.cpp
  panels/inspector_panel.cpp
  panels/scene_view_panel.cpp
  panels/content_browser_panel.cpp
  panels/performance_panel.cpp
)

target_link_libraries(
  signum_editor
  PRIVATE
    signum_engine
    signum_editor_imgui
)
