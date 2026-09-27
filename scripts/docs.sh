#!/usr/bin/env bash
set -euo pipefail

if [[ ! -f Doxyfile ]]; then
    echo "Doxyfile not found."
    exit 1
fi

doxygen Doxyfile
