#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"

preset="${1:-debug}"

if (( $# > 0 )); then
    shift
fi

cd "${project_root}"

"${script_dir}/configure.sh" "${preset}"

if (( $# > 0 )); then
    cmake --build \
        --preset "${preset}" \
        --parallel \
        --target "$@"
else
    cmake --build \
        --preset "${preset}" \
        --parallel
fi

