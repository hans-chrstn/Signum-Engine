#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"
docs_output_dir="${project_root}/build/docs/doxygen"
doxyfile="${project_root}/Doxyfile"

if [[ ! -f "${doxyfile}" ]]; then
    printf 'Doxyfile not found: %s\n' "${doxyfile}"
    exit 1
fi

mkdir -p -- "${docs_output_dir}"

cd -- "${project_root}"
doxygen "${doxyfile}"
