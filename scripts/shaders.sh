#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"

slang_shader_source_dir="${project_root}/shaders/slang/basic"
shader_source_dir="${project_root}/shaders/basic"
shader_output_dir="${project_root}/build/shaders"

mkdir -p -- "${shader_output_dir}"

glslc \
    "${shader_source_dir}/blackhole.vert" \
    -o "${shader_output_dir}/blackhole.vert.spv"

glslc \
    "${shader_source_dir}/blackhole.frag" \
    -o "${shader_output_dir}/blackhole.frag.spv"

spirv-val "${shader_output_dir}/blackhole.vert.spv"
spirv-val "${shader_output_dir}/blackhole.frag.spv"

slangc "${slang_shader_source_dir}/triangle.slang" \
    -target spirv \
    -entry vertexMain \
    -stage vertex \
    -matrix-layout-column-major \
    -o "${shader_output_dir}/triangle.vert.spv"

spirv-val "${shader_output_dir}/triangle.vert.spv"

shader_disassembly="$(spirv-dis "${shader_output_dir}/triangle.vert.spv")"

if [[ "$shader_disassembly" != *"OpMemberDecorate %ModelPushConstants_std430 0 RowMajor"* ]]; then
    echo "Unexpected model matrix layout" >&2
    exit 1
fi

if [[ "$shader_disassembly" != *"OpMemberDecorate %ModelPushConstants_std430 0 MatrixStride 16"* ]]; then
    echo "Unexpected model matrix stride" >&2
    exit 1
fi

if [[ "$shader_disassembly" != *"OpMemberDecorate %ModelPushConstants_std430 0 Offset 0"* ]]; then
    echo "Unexpected model matrix offset" >&2
    exit 1
fi

if [[ "$shader_disassembly" != *"OpVectorTimesMatrix"* ]]; then
    echo "Unexpected model matrix multiplication" >&2
    exit 1
fi

slangc "${slang_shader_source_dir}/triangle.slang" \
    -target spirv \
    -entry fragmentMain \
    -stage fragment \
    -matrix-layout-column-major \
    -o "${shader_output_dir}/triangle.frag.spv"

spirv-val "${shader_output_dir}/triangle.frag.spv"

printf 'Compiled and validated shaders: %s\n' "${shader_output_dir}"
