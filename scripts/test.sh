#!/usr/bin/env bash

set -euo pipefail

preset="${1:-debug}"
scope="${2:-all}"

"$(dirname "$0")/build.sh" "${preset}"

case "${scope}" in
    all)
        ctest --preset "${preset}"
        ;;
    unit)
        ctest --preset "${preset}" -L "^unit$"
        ;;
    integration)
        ctest --preset "${preset}" -L "^integration$"
        ;;
    *)
        echo "Unknown test scope: ${scope}" >&2
        echo "Expected: all, unit, or integration" >&2
        exit 1
        ;;
esac
