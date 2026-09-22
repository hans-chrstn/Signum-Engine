#!/usr/bin/env bash

set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1; then
  echo "clang-format is required."
  exit 1
fi

mapfile -d '' files < <(
  find src tests \
    -type f \
    \( \
      -name '*.cpp' -o \
      -name '*.hpp' -o \
      -name '*.cc' -o \
      -name '*.hh' \
    \) \
    -print0
)

if (( ${#files[@]} == 0 )); then
  exit 0
fi

clang-format -i "${files[@]}"
