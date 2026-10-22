#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUv;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outUv;

layout(push_constant) uniform TransformPushConstants {
  mat4 mvp;
} transform;

layout(set = 1, binding = 2, std140) uniform MaterialParameters {
  vec4 uvOffset;
} material;

void main() {
  gl_Position = transform.mvp * vec4(inPosition, 1.0);

  outNormal = inNormal;
  outUv = inUv + material.uvOffset.xy * 0.01;
}
