#!/usr/bin/env bash

set -euo pipefail

required_commands=(
  cmake
  ninja
  just
  c++
)

missing=0

for command_name in "${required_commands[@]}"; do
  if ! command -v "${command_name}" >/dev/null 2>&1; then
    echo "Missing required command: ${command_name}"
    missing=1
  fi
done

if (( missing != 0 )); then
  exit 1
fi

echo
echo "Toolchain:"
cmake --version | head -n 1
ninja --version
just --version
c++ --version | head -n 1

echo
echo "Default C++ language level:"
c++ -dM -E -x c++ /dev/null | grep __cplusplus || true

echo
echo "Environment looks usable."
