# Signum Engine

A C++26 game engine built around Vulkan 1.4, with a focus on explicit resource ownership, modular architecture, and modern C++ development.

[![CI](https://github.com/hans-chrstn/Signum-Engine/actions/workflows/ci.yml/badge.svg)](https://github.com/hans-chrstn/Signum-Engine/actions/workflows/ci.yml)
[![Vulkan Integration](https://github.com/hans-chrstn/Signum-Engine/actions/workflows/vulkan-integration.yml/badge.svg)](https://github.com/hans-chrstn/Signum-Engine/actions/workflows/vulkan-integration.yml)

> [!WARNING]
> Signum Engine is under active development and is not yet ready for general-purpose game development. APIs, architecture, and features may change significantly.

## Requirements

Signum Engine currently targets Linux (x86-64).

The recommended development environment is managed through Nix.

Core technologies:

- C++26
- GCC 16
- CMake 3.28+
- Ninja
- Vulkan 1.4
- GLFW 3.4+
- Vulkan Memory Allocator
- SPIRV-Reflect
- SPIRV-Tools

Development and testing tools:

- GoogleTest
- Google Benchmark
- Dear ImGui (docking branch)
- Slang and GLSL shader tooling
- LLVM/Clang tools
- Doxygen
- Valgrind
- Weston
- jq
- just

## Development Environment

Enter the Nix development shell:

```bash
nix develop
```

When using direnv with a locally configured `.envrc`:

```bash
direnv allow
```

Verify the development environment:

```bash
just doctor
```

## Nix Packaging

Build the application through Nix:

```bash
nix build .#default
```

The package produces the standalone `signum` executable.

Run the packaged application:

```bash
nix run .
```

## Command Reference

| Command                               | Description                     |
| ------------------------------------- | ------------------------------- |
| `just doctor`                         | Verify development tools        |
| `just configure`                      | Configure the Debug preset      |
| `just build`                          | Build the full Debug workspace  |
| `just build-target <preset> <target>` | Build a specific target         |
| `just release`                        | Build the Release preset        |
| `just run`                            | Run the standalone application  |
| `just editor`                         | Build and launch the Editor     |
| `just playground`                     | Build and launch the Playground |
| `just shaders`                        | Compile precompiled shaders     |
| `just test`                           | Run all tests                   |
| `just test-unit`                      | Run unit tests                  |
| `just test-integration`               | Run integration tests           |
| `just asan`                           | Run sanitized tests             |
| `just memcheck`                       | Run Valgrind                    |
| `just tidy`                           | Run Clang-Tidy                  |
| `just format`                         | Format source files             |
| `just check`                          | Run standard quality checks     |
| `just bench`                          | Run mathematics benchmarks      |
| `just docs`                           | Generate documentation          |
| `just docs-open`                      | Open generated documentation    |
| `just clean`                          | Remove build output             |
| `just all`                            | Run full local verification     |

> Commands supporting presets accept an optional preset name. For example, just build experimental, just test-unit engine, or just editor debug. Run cmake --list-presets to see available configurations.

## License

Licensed under the Apache License, Version 2.0.

See [LICENSE](LICENSE) for details.
