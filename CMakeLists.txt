cmake_minimum_required(VERSION 3.28)

project(
  SignumEngine
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(
  SIGNUM_ENABLE_SANITIZERS
  "Enable AddressSanitizer and UndefinedBehaviorSanitizer"
  OFF
)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.4 REQUIRED)

add_library(signum_project_options INTERFACE)

target_compile_features(
  signum_project_options
  INTERFACE
    cxx_std_23
)

if(MSVC)
  target_compile_options(
    signum_project_options
    INTERFACE
      /W4
      /permissive-
  )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(
    signum_project_options
    INTERFACE
      -Wall
      -Wextra
      -Wpedantic
      -Wconversion
      -Wsign-conversion
  )
endif()

if(SIGNUM_ENABLE_SANITIZERS)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
      signum_project_options
      INTERFACE
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )

    target_link_options(
      signum_project_options
      INTERFACE
        -fsanitize=address,undefined
    )
  endif()
endif()

add_subdirectory(src)

include(CTest)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
