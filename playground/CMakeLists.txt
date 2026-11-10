if(NOT TARGET signum_shader_compiler)
  message(
    FATAL_ERROR
    "Signum Playground requires runtime Slang compilation"
  )
endif()

if(NOT TARGET signum_shaders)
  message(
    FATAL_ERROR
    "Signum Playground requires precompiled shader targets"
  )
endif()

add_executable(
  signum_playground
  main.cpp
)

target_link_libraries(
  signum_playground
  PRIVATE
    signum_engine
    signum_shader_compiler
)

add_dependencies(
  signum_playground
  signum_shaders
)

