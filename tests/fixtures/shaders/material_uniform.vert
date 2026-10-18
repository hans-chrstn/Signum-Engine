#version 450

layout(set = 0, binding = 0, std140) uniform MaterialParameters {
  vec4 offset;
} parameters;

void main() {
  gl_Position = vec4(parameters.offset.xyz, 1.0);
}
