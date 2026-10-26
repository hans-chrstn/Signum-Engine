#!/usr/bin/env bash

set -euo pipefail

preset="${1:-debug}"
scope="${2:-all}"
display_mode="${SIGNUM_TEST_DISPLAY:-headless}"

"$(dirname "$0")/build.sh" "${preset}"

ctest_args=(--preset "${preset}")

case "${scope}" in
    all)
        ;;
    unit)
        ctest_args+=(-L "^unit$")
        ;;
    integration)
        ctest_args+=(-L "^integration$")
        ;;
    *)
        echo "Unknown test scope: ${scope}" >&2
        echo "Expected: all, unit, or integration" >&2
        exit 1
        ;;
esac

case "${display_mode}" in
    native | headless)
        ;;
    *)
        echo "Unknown test display mode: ${display_mode}" >&2
        echo "Expected: headless or native" >&2
        exit 1
        ;;
esac

if [[ "${scope}" == "unit" || "${display_mode}" == "native" ]]; then
    exec ctest "${ctest_args[@]}"
fi

if ! command -v weston >/dev/null 2>&1; then
    echo "Weston is required for headless integration testing" >&2
    exit 1
fi

if [[ -z "${XDG_RUNTIME_DIR:-}" || ! -d "${XDG_RUNTIME_DIR}" ]]; then
    echo "Headless testing requires a valid XDG_RUNTIME_DIR" >&2
    exit 1
fi

weston_socket="signum-test-$$"
weston_log="${TMPDIR:-/tmp}/signum-weston-$$.log"

weston \
    --backend=headless \
    --renderer=gl \
    --fake-seat \
    --socket="${weston_socket}" \
    --idle-time=0 \
    --log="${weston_log}" \
    >/dev/null 2>&1 &

weston_pid=$!

cleanup() {
    local result=$?

    trap - EXIT

    kill "${weston_pid}" 2>/dev/null || true
    wait "${weston_pid}" 2>/dev/null || true

    if (( result == 0 )); then
        rm -f -- "${weston_log}"
    else
        echo "Weston log: ${weston_log}" >&2
    fi
}

trap cleanup EXIT
trap 'exit 130' INT
trap 'exit 143' TERM
trap 'exit 129' HUP

weston_ready=false

for ((attempt = 0; attempt < 100; ++attempt)); do
    if ! kill -0 "${weston_pid}" 2>/dev/null; then
        echo "Weston exited during startup" >&2
        cat "${weston_log}" >&2
        exit 1
    fi

    if [[ -S "${XDG_RUNTIME_DIR}/${weston_socket}" ]]; then
        weston_ready=true
        break
    fi

    sleep 0.1
done

if [[ "${weston_ready}" != true ]]; then
    echo "Weston failed to create its Wayland socket" >&2
    cat "${weston_log}" >&2
    exit 1
fi

echo "Running Signum tests using isolated Weston display"
echo "Display: ${weston_socket}"

env -u DISPLAY -u WAYLAND_SOCKET \
    WAYLAND_DISPLAY="${weston_socket}" \
    XDG_SESSION_TYPE=wayland \
    ctest "${ctest_args[@]}"
