#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec2 outUv;

layout(push_constant) uniform TransformPushConstants {
  mat4 mvp;
} transform;

layout(set = 1, binding = 2, std140) uniform MaterialParameters {
  vec4 uvOffset;
} material;

void main() {
  gl_Position = transform.mvp * vec4(inPosition, 1.0);
  outUv = inUv + material.uvOffset.xy * 0.01;
}
