#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

"${script_dir}/doctor.sh"
"${script_dir}/format-check.sh"
"${script_dir}/test.sh" debug
