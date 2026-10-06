#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"

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

printf 'Compiled and validated shaders: %s\n' "${shader_output_dir}"
