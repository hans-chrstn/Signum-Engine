
#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd -- "${script_dir}/.." && pwd)"

cd "${project_root}"

"${script_dir}/configure.sh" bench

cmake --build --preset bench \
    --target signum_math_benchmarks \
    --parallel

"${project_root}/build/bench/benchmarks/signum_math_benchmarks" "$@"

