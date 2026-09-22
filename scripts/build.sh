#!/usr/bin/env bash

set -euo pipefail

preset="${1:-debug}"

"$(dirname "$0")/configure.sh" "${preset}"

cmake --build --preset "${preset}" --parallel
