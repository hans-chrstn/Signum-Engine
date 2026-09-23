#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"
test_dir="${project_root}/build/debug/tests/unit"
test_binary="${test_dir}/signum_unit_tests"

if ! command -v valgrind >/dev/null 2>&1; then
    echo "valgrind is required."
    exit 1
fi

if [[ ! -x "${test_binary}" ]]; then
    echo "Test executable not found. Run 'just build' first."
    exit 1
fi

echo "Running Signum unit tests through Valgrind Memcheck..."

cd "${test_dir}"

valgrind \
    --tool=memcheck \
    --leak-check=full \
    --show-leak-kinds=definite,possible \
    --errors-for-leak-kinds=definite \
    --error-exitcode=1 \
    "${test_binary}"

echo "Valgrind Memcheck completed successfully."
