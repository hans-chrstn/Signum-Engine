{
  lib,
  stdenv,
  cmake,
  ninja,
  pkg-config,
  glfw,
  vulkan-loader,
  vulkan-headers,
  vulkan-memory-allocator,
  spirv-reflect,
  spirv-tools,
  spirv-headers,
  stb,
}:
stdenv.mkDerivation {
  pname = "signum-engine";
  version = "0.1.0-dev";

  src = ../.;

  nativeBuildInputs = [
    cmake
    ninja
    pkg-config
  ];

  buildInputs = [
    glfw
    vulkan-loader
    vulkan-headers
    vulkan-memory-allocator
    spirv-tools
    spirv-headers
    stb
  ];

  cmakeFlags = [
    "-DCMAKE_BUILD_TYPE=Release"

    "-DBUILD_TESTING=OFF"
    "-DSIGNUM_BUILD_EDITOR=OFF"
    "-DSIGNUM_BUILD_PLAYGROUND=OFF"
    "-DSIGNUM_BUILD_BENCHMARKS=OFF"
    "-DSIGNUM_BUILD_SHADERS=OFF"

    "-DSIGNUM_ENABLE_RUNTIME_SHADER_COMPILATION=OFF"
    "-DSIGNUM_ENABLE_SANITIZERS=OFF"
    "-DSIGNUM_ENABLE_REFLECTION=OFF"
    "-DSIGNUM_ENABLE_CONTRACTS=OFF"

    "-DSPIRV_REFLECT_SOURCE_DIR=${spirv-reflect}"
  ];

  meta = {
    description = "C++26 game engine built around Vulkan";
    homepage = "https://github.com/hans-chrstn/Signum-Engine";
    license = lib.licenses.asl20;
    platforms = lib.platforms.linux;
    mainProgram = "signum";
  };
}
