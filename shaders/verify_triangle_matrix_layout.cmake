if(NOT DEFINED SPIRV_DIS_EXECUTABLE OR NOT DEFINED SPIRV_FILE)
  message(FATAL_ERROR "Triangle shader verification requires the disassembler and SPIR-V file paths")
endif()

execute_process(
  COMMAND
    "${SPIRV_DIS_EXECUTABLE}"
    "${SPIRV_FILE}"

  RESULT_VARIABLE disassembly_result
  OUTPUT_VARIABLE shader_disassembly
  ERROR_VARIABLE disassembly_error
)

if(NOT disassembly_result EQUAL 0)
  message(
    FATAL_ERROR
    "Failed to disassemble triangle vertex shader: ${disassembly_error}"
  )
endif()

set(
  required_patterns
  "OpMemberDecorate %TransformPushConstants_std430 0 RowMajor"
  "OpMemberDecorate %TransformPushConstants_std430 0 MatrixStride 16"
  "OpMemberDecorate %TransformPushConstants_std430 0 Offset 0"
  "OpVectorTimesMatrix"
)

foreach(pattern IN LISTS required_patterns)
  string(
    FIND
    "${shader_disassembly}"
    "${pattern}"
    pattern_position
  )

  if(pattern_position EQUAL -1)
    message(
      FATAL_ERROR
      "Triangle vertex shader matrix-layout regression: "
      "expected SPIR-V disassembly to contain '${pattern}'"
    )
  endif()
endforeach()

message(STATUS "Triangle vertex shader matrix-layout checks passed")

