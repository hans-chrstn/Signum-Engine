#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"

preset="${1:-debug}"

cd "${project_root}"

"${script_dir}/build.sh" "${preset}" signum

exec "./build/${preset}/bin/signum"
