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

option(
  SIGNUM_WARNINGS_AS_ERRORS
  "Treat compiler warnings as errors"
  OFF
)

find_package(Vulkan REQUIRED)
find_package(glfw3 3.4 REQUIRED)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(PkgConfig REQUIRED)

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

add_subdirectory(src)

include(CTest)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

if(SIGNUM_BUILD_BENCHMARKS)
  add_subdirectory(benchmarks)
endif()
