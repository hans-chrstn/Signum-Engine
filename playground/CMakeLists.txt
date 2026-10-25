add_executable(
  signum_playground
  main.cpp
)

target_link_libraries(
  signum_playground
  PRIVATE
    signum_engine
)

if(SIGNUM_ENABLE_RUNTIME_SHADER_COMPILATION)
  target_link_libraries(
    signum_playground
    PRIVATE
      signum_shader_compiler
  )
endif()
