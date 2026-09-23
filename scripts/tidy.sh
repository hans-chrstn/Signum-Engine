#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"

cd "${project_root}"

if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "clang-tidy is required."
    exit 1
fi

if [[ ! -f build/debug/compile_commands.json ]]; then
    echo "Missing compilation database. Run 'just build' first."
    exit 1
fi

mapfile -d '' sources < <(
    find src tests -type f -name '*.cpp' -print0
)

if (( ${#sources[@]} == 0 )); then
    exit 0
fi

clang-tidy --quiet -p build/debug "${sources[@]}"
