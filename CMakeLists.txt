cmake_minimum_required(VERSION 3.28)

project(
  SignumEngine
  VERSION 0.1.0
  LANGUAGES C CXX
)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

option(
  SIGNUM_ENABLE_SANITIZERS
  "Enable AddressSanitizer and UndefinedBehaviorSanitizer"
  OFF
)

option(
  SIGNUM_WARNINGS_AS_ERRORS
  "Treat compiler warnings as errors"
  OFF
)

option(
  SIGNUM_ENABLE_REFLECTION
  "Enable experimental C++26 reflection"
  OFF
)

option(
  SIGNUM_ENABLE_CONTRACTS
  "Enable experimental C++26 contracts"
  OFF
)

option(
  SIGNUM_BUILD_BENCHMARKS
  "Build standalone mathematics benchmarks"
  OFF
)

option(
  SIGNUM_BUILD_PLAYGROUND
  "Build the optional Signum Playground application"
  OFF
)

option(
  SIGNUM_BUILD_EDITOR
  "Build the optional Signum Editor application"
  OFF
)

option(
  SIGNUM_BUILD_SHADERS
  "Build the shaders"
  OFF
)

option(
  SIGNUM_ENABLE_RUNTIME_SHADER_COMPILATION
  "Enable runtime shader source compilation"
  OFF
)



find_package(Vulkan REQUIRED)
find_package(glfw3 3.4 REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)

if(SIGNUM_ENABLE_RUNTIME_SHADER_COMPILATION)
  find_package(slang CONFIG REQUIRED)
endif()

find_path(
  SIGNUM_STB_INCLUDE_DIR
  NAMES stb_image.h
  PATH_SUFFIXES stb
  REQUIRED
)

pkg_check_modules(
  SPIRVTools
  REQUIRED
  IMPORTED_TARGET
  SPIRV-Tools
)

add_library(signum_project_options INTERFACE)

target_compile_features(
  signum_project_options
  INTERFACE
    cxx_std_26
)

if(SIGNUM_ENABLE_REFLECTION OR SIGNUM_ENABLE_CONTRACTS)
  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(
      FATAL_ERROR
      "Experimental reflection/contracts currently require GCC"
    )
  endif()

  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16)
    message(
      FATAL_ERROR
      "Experimental reflection/contracts require GCC 16 or newer"
    )
  endif()
endif()

if(SIGNUM_ENABLE_REFLECTION)
  target_compile_options(
    signum_project_options
    INTERFACE
      -freflection
  )
endif()

if(SIGNUM_ENABLE_CONTRACTS)
  target_compile_options(
    signum_project_options
    INTERFACE
      -fcontracts
  )
endif()

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

if(SIGNUM_WARNINGS_AS_ERRORS)
  if(MSVC)
    target_compile_options(
      signum_project_options
      INTERFACE
        /WX
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(
      signum_project_options
      INTERFACE
        -Werror
    )
  endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  target_compile_options(
    signum_project_options
    INTERFACE
      -Wdocumentation
      -Wdocumentation-pedantic
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

set(
  SPIRV_REFLECT_SOURCE_DIR
  "$ENV{SPIRV_REFLECT_SOURCE_DIR}"
  CACHE PATH "Path to SPIRV-Reflect source"
)

if(NOT EXISTS "${SPIRV_REFLECT_SOURCE_DIR}/spirv_reflect.c")
  message(FATAL_ERROR "SPIRV-Reflect source directory is unavailable")
endif()

set(SPIRV_REFLECT_EXECUTABLE OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_STATIC_LIB ON CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_SHARED_LIB OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_REFLECT_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(
  "${SPIRV_REFLECT_SOURCE_DIR}"
  "${CMAKE_BINARY_DIR}/third_party/spirv-reflect"
)

add_subdirectory(src)

if(SIGNUM_BUILD_PLAYGROUND)
  add_subdirectory(playground)
endif()

if(SIGNUM_BUILD_EDITOR)
  add_subdirectory(editor)
endif()

if(SIGNUM_BUILD_SHADERS)
  add_subdirectory(
    shaders
    "${CMAKE_BINARY_DIR}/cmake/shaders"
  )
endif()

include(CTest)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(SIGNUM_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
