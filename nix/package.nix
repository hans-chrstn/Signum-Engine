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
  ];

  cmakeFlags = [
    "-DBUILD_TESTING=OFF"
    "-DSIGNUM_ENABLE_SANITIZERS=OFF"
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
