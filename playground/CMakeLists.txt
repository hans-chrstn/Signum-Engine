add_executable(
  signum_playground
  main.cpp
)

target_link_libraries(
  signum_playground
  PRIVATE
    signum_engine
)
