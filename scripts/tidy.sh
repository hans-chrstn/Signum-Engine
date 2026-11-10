#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"

preset="${1:-debug}"
build_dir="${project_root}/build/${preset}"
compile_commands="${build_dir}/compile_commands.json"

jobs="${SIGNUM_TIDY_JOBS:-4}"

cd "${project_root}"

if ! command -v clang-tidy >/dev/null 2>&1; then
    echo "clang-tidy is required." >&2
    exit 1
fi

if ! command -v jq >/dev/null 2>&1; then
    echo "jq is required." >&2
    exit 1
fi

if [[ ! -f "${compile_commands}" ]]; then
    echo "Compilation database not found: ${compile_commands}" >&2
    echo "Configure the preset first." >&2
    exit 1
fi

if ! [[ "${jobs}" =~ ^[1-9][0-9]*$ ]]; then
    echo "SIGNUM_TIDY_JOBS must be a positive integer." >&2
    exit 1
fi

echo "Running Clang-Tidy with preset: ${preset}"
echo "Parallel jobs: ${jobs}"

jq -j \
    --arg root "${project_root}/" \
    '
    .[]
    | .file
    | select(startswith($root))
    | select(
        startswith($root + "src/") or
        startswith($root + "tests/") or
        startswith($root + "editor/") or
        startswith($root + "playground/") or
        startswith($root + "benchmarks/")
      )
    | select(test("\\.(cc|cpp|cxx)$"))
    | ., "\u0000"
    ' \
    "${compile_commands}" |
    sort -zu |
    xargs -0 -r -n 1 -P "${jobs}" \
        clang-tidy --quiet -p "${build_dir}"

