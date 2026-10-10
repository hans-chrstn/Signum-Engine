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
    gccStdenv = pkgs.gcc16Stdenv;
    clangToolsBase = llvm.clang-tools.override {
      enableLibcxx = false;
    };

    clangTools = pkgs.lib.hiPrio (clangToolsBase.overrideAttrs (oldAttrs: {
      postInstall =
        (oldAttrs.postInstall or "")
        + ''
          for tool in clangd clang-tidy clang-format; do
            substituteInPlace "$out/bin/$tool" \
              --replace-fail '#!/bin/sh' '#!${pkgs.bash}/bin/bash'
          done
        '';
    }));

    runtimeLibraries = with pkgs; [
      glfw
      libGL
      vulkan-loader
      vulkan-validation-layers
      freetype
    ];

    mkDevShell = extraPackages:
      pkgs.mkShell.override {
        stdenv = gccStdenv;
      } {
        name = "signum-engine-dev";

        packages = with pkgs;
          [
            # Build
            cmake
            ninja
            just
            pkg-config

            # LLVM tooling
            clangTools
            llvm.lldb
            neocmakelsp

            # Development tools
            gdb
            valgrind
            doxygen
            cppcheck
            codespell
            lcov

            # Vulkan tooling
            glslang
            glsl_analyzer
            shader-slang
            spirv-tools
            vulkan-tools
            renderdoc
          ]
          ++ extraPackages;

        buildInputs = with pkgs; [
          # Tests
          gtest
          gbenchmark

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
          g++ --version | head -n 1
          clangd --version | head -n 1
          clang-tidy --version | head -n 1
          cmake --version | head -n 1
          ninja --version
          just --version
        '';
      };
  in {
    packages.${system}.default = pkgs.callPackage ./nix/package.nix {
      stdenv = gccStdenv;
    };

    devShells.${system} = {
      default = mkDevShell [];
      ci = mkDevShell [pkgs.mesa.llvmpipeHook];
    };

    apps.${system}.default = {
      type = "app";
      program = "${self.packages.${system}.default}/bin/signum";
    };
  };
}
