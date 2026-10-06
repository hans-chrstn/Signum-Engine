# Signum Engine

Game engine using C++26 built around Vulkan.

[![CI](https://github.com/hans-chrstn/Signum-Engine/actions/workflows/ci.yml/badge.svg)](https://github.com/hans-chrstn/Signum-Engine/actions/workflows/ci.yml)
[![Vulkan Integration](https://github.com/hans-chrstn/Signum-Engine/actions/workflows/vulkan-integration.yml/badge.svg)](https://github.com/hans-chrstn/Signum-Engine/actions/workflows/vulkan-integration.yml)

> [!WARNING]
> This project is under active development and most game engine features are not implemented so you are best of
> using other engines

## Requirements

The current development environment targets only Linux.

Core requirements:

- C++26
- GCC 16
- CMake 3.28+
- Ninja
- just
- GLFW 3.4+
- Vulkan 1.4
- GoogleTest

Development tooling also uses:

- Clang 23 tools (clangd, clang-format, and clang-tidy)
- Doxygen
- Valgrind

## Development Environment

The recommended environment is provided by the repository flake:

```bash
nix develop
```

With direnv:

```bash
direnv allow
```

## Build

Build the default Debug configuration:

```bash
just build
```

Equivalent:

```bash
just build debug
```

Build with the experimental GCC 16 reflection and contracts support:

```bash
just build experimental
```

The regular Debug preset keeps those experimental features disabled so clangd
and clang-tidy can use `build/debug/compile_commands.json`.

## Run

Run the Debug build:

```bash
just run
```

## Tests

Run all tests:

```bash
just test
```

Run only unit tests:

```bash
just test-unit
```

Run only integration tests:

```bash
just test-integration
```

## Code Quality

Check formatting:

```bash
./scripts/format-check.sh
```

Apply formatting:

```bash
just format
```

Run Clang-Tidy:

```bash
just tidy
```

Run the standard development checks:

```bash
just check
```

## Documentation

Generate the Doxygen documentation:

```bash
just docs
```

Open the generated documentation:

```bash
just docs-open
```

Generated documentation is written to:

```text
build/docs/doxygen/html/
```

## Useful Commands

```text
just doctor             Check the local development environment
just configure          Configure the Debug preset
just build              Build the Debug preset
just build experimental Build with experimental reflection and contracts
just release            Build the Release preset
just run                Run the Debug executable
just run experimental   Run the experimental executable
just test               Run all tests
just test experimental  Run tests with reflection and contracts enabled
just test-unit          Run unit tests
just test-integration   Run Vulkan integration tests
just asan               Run tests with ASan and UBSan
just memcheck           Run Valgrind
just tidy               Run Clang-Tidy
just format             Format source files
just docs               Generate documentation
just clean              Remove build output
just all                Run the full local verification workflow
```

## License

Licensed under the Apache License, Version 2.0. See [`LICENSE`](LICENSE) for details.
