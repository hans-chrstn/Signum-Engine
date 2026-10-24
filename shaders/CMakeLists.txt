find_program(
  SIGNUM_SLANGC_EXECUTABLE
  NAMES slangc
  REQUIRED
)

find_program(
  SIGNUM_SPIRV_VAL_EXECUTABLE
  NAMES spirv-val
  REQUIRED
)

find_program(
  SIGNUM_GLSLC_EXECUTABLE
  NAMES glslc
  REQUIRED
)

find_program(
  SIGNUM_SPIRV_DIS_EXECUTABLE
  NAMES spirv-dis
  REQUIRED
)

set(
  SIGNUM_SHADER_SOURCE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/slang/basic"
)

set(
  SIGNUM_SLANG_TEST_SOURCE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/slang/tests"
)

set(
  SIGNUM_SHADER_OUTPUT_DIR
  "${PROJECT_SOURCE_DIR}/build/shaders"
)

set(
  SIGNUM_GLSL_BASIC_SOURCE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/glsl/basic"
)

set(
  SIGNUM_GLSL_TEST_SOURCE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/glsl/tests"
)

function(signum_compile_slang_shader output_path source_file entry_point stage)
  add_custom_command(
    OUTPUT "${output_path}"

    COMMAND
      "${CMAKE_COMMAND}" -E make_directory
      "${SIGNUM_SHADER_OUTPUT_DIR}"

    COMMAND
      "${SIGNUM_SLANGC_EXECUTABLE}"
      "${source_file}"
      -target spirv
      -entry "${entry_point}"
      -stage "${stage}"
      -matrix-layout-column-major
      -o "${output_path}"

    COMMAND
      "${SIGNUM_SPIRV_VAL_EXECUTABLE}"
      "${output_path}"

    DEPENDS
      "${source_file}"

    COMMENT "Compiling and validating ${entry_point} (${stage})"

    VERBATIM
  )
endfunction()

function(signum_compile_glsl_shader output_path source_file)
  add_custom_command(
    OUTPUT "${output_path}"

    COMMAND
      "${CMAKE_COMMAND}" -E make_directory
      "${SIGNUM_SHADER_OUTPUT_DIR}"

    COMMAND
      "${SIGNUM_GLSLC_EXECUTABLE}"
      "${source_file}"
      -o "${output_path}"

    COMMAND
      "${SIGNUM_SPIRV_VAL_EXECUTABLE}"
      "${output_path}"

    DEPENDS
      "${source_file}"

    COMMENT "Compiling and validating GLSL shader"

    VERBATIM
  )
endfunction()

set(
  SIGNUM_EARTH_FRAGMENT_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/earth.frag.spv"
)

set(
  SIGNUM_TEXTURED_VERTEX_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/textured.vert.spv"
)

set(
  SIGNUM_MATERIAL_UPDATE_VERTEX_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/material_update.vert.spv"
)

set(
  SIGNUM_MATERIAL_STORAGE_VERTEX_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/material_storage.vert.spv"
)

set(
  SIGNUM_MATERIAL_TEXTURED_VERTEX_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/material_textured.vert.spv"
)

set(
  SIGNUM_MATERIAL_MIXED_FRAGMENT_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/material_mixed.frag.spv"
)

set(
  SIGNUM_BLACKHOLE_VERTEX_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/blackhole.vert.spv"
)

set(
  SIGNUM_BLACKHOLE_FRAGMENT_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/blackhole.frag.spv"
)

set(
  SIGNUM_TRIANGLE_VERTEX_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/triangle.vert.spv"
)

set(
  SIGNUM_TRIANGLE_FRAGMENT_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/triangle.frag.spv"
)

set(
  SIGNUM_TEXTURED_FRAGMENT_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/textured.frag.spv"
)

set(
  SIGNUM_MATERIAL_TWO_TEXTURES_FRAGMENT_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/material_two_textures.frag.spv"
)

set(
  SIGNUM_MATRIX_TRANSFORM_COMPUTE_SPIRV
  "${SIGNUM_SHADER_OUTPUT_DIR}/matrix_transform.comp.spv"
)

signum_compile_slang_shader(
  "${SIGNUM_EARTH_FRAGMENT_SPIRV}"
  "${SIGNUM_SHADER_SOURCE_DIR}/earth.slang"
  fragmentMain
  fragment
)

signum_compile_slang_shader(
  "${SIGNUM_TEXTURED_VERTEX_SPIRV}"
  "${SIGNUM_SHADER_SOURCE_DIR}/textured.slang"
  vertexMain
  vertex
)

signum_compile_glsl_shader(
  "${SIGNUM_MATERIAL_UPDATE_VERTEX_SPIRV}"
  "${SIGNUM_GLSL_TEST_SOURCE_DIR}/material_update.vert"
)

signum_compile_glsl_shader(
  "${SIGNUM_MATERIAL_STORAGE_VERTEX_SPIRV}"
  "${SIGNUM_GLSL_TEST_SOURCE_DIR}/material_storage.vert"
)

signum_compile_glsl_shader(
  "${SIGNUM_MATERIAL_TEXTURED_VERTEX_SPIRV}"
  "${SIGNUM_GLSL_TEST_SOURCE_DIR}/material_textured.vert"
)

signum_compile_glsl_shader(
  "${SIGNUM_MATERIAL_MIXED_FRAGMENT_SPIRV}"
  "${SIGNUM_GLSL_TEST_SOURCE_DIR}/material_mixed.frag"
)

signum_compile_glsl_shader(
  "${SIGNUM_BLACKHOLE_VERTEX_SPIRV}"
  "${SIGNUM_GLSL_BASIC_SOURCE_DIR}/blackhole.vert"
)

signum_compile_glsl_shader(
  "${SIGNUM_BLACKHOLE_FRAGMENT_SPIRV}"
  "${SIGNUM_GLSL_BASIC_SOURCE_DIR}/blackhole.frag"
)

signum_compile_slang_shader(
  "${SIGNUM_TRIANGLE_VERTEX_SPIRV}"
  "${SIGNUM_SHADER_SOURCE_DIR}/triangle.slang"
  vertexMain
  vertex
)

signum_compile_slang_shader(
  "${SIGNUM_TRIANGLE_FRAGMENT_SPIRV}"
  "${SIGNUM_SHADER_SOURCE_DIR}/triangle.slang"
  fragmentMain
  fragment
)

signum_compile_slang_shader(
  "${SIGNUM_TEXTURED_FRAGMENT_SPIRV}"
  "${SIGNUM_SHADER_SOURCE_DIR}/textured.slang"
  fragmentMain
  fragment
)

signum_compile_slang_shader(
  "${SIGNUM_MATERIAL_TWO_TEXTURES_FRAGMENT_SPIRV}"
  "${SIGNUM_SLANG_TEST_SOURCE_DIR}/material_two_textures.slang"
  fragmentMain
  fragment
)

signum_compile_slang_shader(
  "${SIGNUM_MATRIX_TRANSFORM_COMPUTE_SPIRV}"
  "${SIGNUM_SLANG_TEST_SOURCE_DIR}/matrix_transform.slang"
  computeMain
  compute
)


set(
  SIGNUM_TRIANGLE_MATRIX_LAYOUT_STAMP
  "${SIGNUM_SHADER_OUTPUT_DIR}/triangle_matrix_layout.verified"
)

add_custom_command(
  OUTPUT "${SIGNUM_TRIANGLE_MATRIX_LAYOUT_STAMP}"

  COMMAND
    "${CMAKE_COMMAND}" -E make_directory
    "${SIGNUM_SHADER_OUTPUT_DIR}"

  COMMAND
    "${CMAKE_COMMAND}"
    "-DSPIRV_DIS_EXECUTABLE=${SIGNUM_SPIRV_DIS_EXECUTABLE}"
    "-DSPIRV_FILE=${SIGNUM_TRIANGLE_VERTEX_SPIRV}"
    -P
    "${CMAKE_CURRENT_SOURCE_DIR}/verify_triangle_matrix_layout.cmake"

  COMMAND
    "${CMAKE_COMMAND}" -E touch
    "${SIGNUM_TRIANGLE_MATRIX_LAYOUT_STAMP}"

  DEPENDS
    "${SIGNUM_TRIANGLE_VERTEX_SPIRV}"
    "${CMAKE_CURRENT_SOURCE_DIR}/verify_triangle_matrix_layout.cmake"

  COMMENT "Verifying triangle vertex shader matrix layout"

  VERBATIM
)

add_custom_target(
  signum_shaders
  ALL
  DEPENDS
    "${SIGNUM_EARTH_FRAGMENT_SPIRV}"
    "${SIGNUM_TEXTURED_VERTEX_SPIRV}"
    "${SIGNUM_MATERIAL_UPDATE_VERTEX_SPIRV}"
    "${SIGNUM_MATERIAL_STORAGE_VERTEX_SPIRV}"
    "${SIGNUM_MATERIAL_TEXTURED_VERTEX_SPIRV}"
    "${SIGNUM_MATERIAL_MIXED_FRAGMENT_SPIRV}"
    "${SIGNUM_BLACKHOLE_VERTEX_SPIRV}"
    "${SIGNUM_BLACKHOLE_FRAGMENT_SPIRV}"
    "${SIGNUM_TRIANGLE_VERTEX_SPIRV}"
    "${SIGNUM_TRIANGLE_FRAGMENT_SPIRV}"
    "${SIGNUM_TEXTURED_FRAGMENT_SPIRV}"
    "${SIGNUM_MATERIAL_TWO_TEXTURES_FRAGMENT_SPIRV}"
    "${SIGNUM_MATRIX_TRANSFORM_COMPUTE_SPIRV}"
    "${SIGNUM_TRIANGLE_MATRIX_LAYOUT_STAMP}"
)
