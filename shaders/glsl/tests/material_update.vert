#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 outColor;

layout(push_constant) uniform TransformPushConstants {
  mat4 mvp;
} transform;

layout(set = 1, binding = 2, std140) uniform MaterialParameters {
  vec4 colorMultiplier;
} material;

void main() {
  gl_Position = transform.mvp * vec4(inPosition, 1.0);
  outColor = inColor * material.colorMultiplier.rgb;
}
