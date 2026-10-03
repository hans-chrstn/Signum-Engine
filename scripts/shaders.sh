#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"

shader_source_dir="${project_root}/shaders/basic"
shader_output_dir="${project_root}/build/shaders"

mkdir -p -- "${shader_output_dir}"

glslc \
    "${shader_source_dir}/triangle.vert" \
    -o "${shader_output_dir}/triangle.vert.spv"

glslc \
    "${shader_source_dir}/triangle.frag" \
    -o "${shader_output_dir}/triangle.frag.spv"

spirv-val "${shader_output_dir}/triangle.vert.spv"
spirv-val "${shader_output_dir}/triangle.frag.spv"

printf 'Compiled and validated shaders: %s\n' "${shader_output_dir}"
