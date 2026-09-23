#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"
build_dir="${project_root}/build"

if [[ -d "${build_dir}" ]]; then
    rm -rf -- "${build_dir}"
    printf 'Removed build directory: %s\n' "${build_dir}"
else
    printf 'Nothing to clean: build directory does not exist.\n'
fi
