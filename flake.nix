{
  description = "Signum Engine development environment";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = {
    self,
    nixpkgs,
    ...
  }: let
    system = "x86_64-linux";

    pkgs = import nixpkgs {
      inherit system;
    };

    llvm = pkgs.llvmPackages_23;

    clangToolsBase = llvm.clang-tools.override {
      enableLibcxx = false;
    };

    clangTools = pkgs.lib.hiPrio (pkgs.symlinkJoin {
      name = "signum-clang-tools";

      paths = [
        clangToolsBase
      ];

      nativeBuildInputs = [
        pkgs.makeWrapper
      ];

      postBuild = ''
        for tool in clangd clang-tidy clang-format; do
          rm -f "$out/bin/$tool"

          makeWrapper ${pkgs.bash}/bin/bash "$out/bin/$tool" \
            --add-flags "${clangToolsBase}/bin/$tool"
        done
      '';
    });

    runtimeLibraries = with pkgs; [
      glfw
      libGL
      vulkan-loader
      vulkan-validation-layers
      freetype
    ];
  in {
    packages.${system}.default = pkgs.callPackage ./nix/package.nix {
      stdenv = llvm.stdenv;
    };

    devShells.${system}.default =
      pkgs.mkShell.override {
        stdenv = llvm.stdenv;
      } {
        name = "signum-engine-dev";

        packages = with pkgs; [
          # Build
          cmake
          ninja
          just
          pkg-config

          # LLVM tooling
          clangTools
          llvm.lldb
          llvm.libstdcxxClang

          # Development tools
          gdb
          valgrind
          doxygen
          cppcheck
          codespell
          lcov

          # Vulkan tooling
          glslang
          spirv-tools
          vulkan-tools
          renderdoc
        ];

        buildInputs = with pkgs; [
          # Tests
          gtest

          # Windowing
          glfw

          # Vulkan
          vulkan-loader
          vulkan-headers
          vulkan-validation-layers

          # Graphics
          libGL
          glm
          shaderc
          vulkan-volk
          vulkan-memory-allocator
          stb
          freetype
        ];

        LD_LIBRARY_PATH =
          pkgs.lib.makeLibraryPath runtimeLibraries;

        VK_ADD_LAYER_PATH = "${pkgs.vulkan-validation-layers}/share/vulkan/explicit_layer.d";

        shellHook = ''
          echo "Signum Engine dev shell"
          echo
          clang++ --version | head -n 1
          clangd --version | head -n 1
          clang-tidy --version | head -n 1
          cmake --version | head -n 1
          ninja --version
          just --version
        '';
      };

    apps.${system}.default = {
      type = "app";
      program = "${self.packages.${system}.default}/bin/signum";
    };
  };
}
