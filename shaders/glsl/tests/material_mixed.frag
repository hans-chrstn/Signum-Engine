#version 450

layout(location = 0) in vec3 inColor;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 3, std140) uniform OccasionalParameters {
  vec4 colorMultiplier;
} occasional;

void main() {
  outColor = vec4(
      inColor * occasional.colorMultiplier.rgb,
      1.0
    );
}
