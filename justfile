set shell := ["bash", "-eu", "-o", "pipefail", "-c"]

default:
    @just --list

doctor:
    ./scripts/doctor.sh

configure preset="debug":
    ./scripts/configure.sh "{{preset}}"

build preset="debug":
    ./scripts/build.sh "{{preset}}"

build-target preset target:
    ./scripts/build.sh "{{preset}}" "{{target}}"

release:
    ./scripts/build.sh release

run preset="debug":
    ./scripts/run.sh "{{preset}}"

editor preset="editor":
    ./scripts/build.sh "{{preset}}" signum_editor
    ./build/{{preset}}/editor/signum_editor

playground preset="playground":
    ./scripts/build.sh "{{preset}}" signum_playground
    ./build/{{preset}}/playground/signum_playground

shaders preset="debug":
    ./scripts/build.sh "{{preset}}" signum_shaders

test preset="debug":
    ./scripts/test.sh "{{preset}}" all

test-unit preset="debug":
    ./scripts/test.sh "{{preset}}" unit

test-integration preset="debug":
    ./scripts/test.sh "{{preset}}" integration

asan:
    ./scripts/test.sh asan all

memcheck:
    ./scripts/build.sh debug signum_unit_tests
    ./scripts/memcheck.sh

tidy:
    ./scripts/configure.sh debug
    ./scripts/tidy.sh

format:
    ./scripts/format.sh

check:
    ./scripts/check.sh

docs:
    ./scripts/docs.sh

docs-open:
    xdg-open build/docs/doxygen/html/index.html

bench:
    ./scripts/bench.sh

clean:
    ./scripts/clean.sh

all:
    ./scripts/clean.sh
    ./scripts/format.sh
    ./scripts/check.sh
    ./scripts/test.sh warnings all
    ./scripts/test.sh asan all
    ./scripts/memcheck.sh

