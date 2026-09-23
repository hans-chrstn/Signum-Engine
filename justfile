set shell := ["bash", "-eu", "-o", "pipefail", "-c"]

default:
    @just --list

doctor:
    ./scripts/doctor.sh

tidy:
    ./scripts/build.sh debug
    ./scripts/tidy.sh

memcheck:
    ./scripts/build.sh debug
    ./scripts/memcheck.sh

configure preset="debug":
    ./scripts/configure.sh "{{preset}}"

build preset="debug":
    ./scripts/build.sh "{{preset}}"

test preset="debug":
    ./scripts/test.sh "{{preset}}"

run preset="debug":
    ./scripts/run.sh "{{preset}}"

format:
    ./scripts/format.sh

check:
    ./scripts/check.sh

asan:
    ./scripts/test.sh asan

release:
    ./scripts/build.sh release

clean:
    ./scripts/clean.sh

all:
    ./scripts/clean.sh
    ./scripts/check.sh
    ./scripts/test.sh asan
