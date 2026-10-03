#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
preset="${1:-debug}"

"${script_dir}/shaders.sh"
"${script_dir}/configure.sh" "${preset}"

cmake --build --preset "${preset}" --parallel
