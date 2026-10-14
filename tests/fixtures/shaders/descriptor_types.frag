#version 450

layout(set = 0, binding = 0) uniform sampler2D textures[3];

layout(set = 1, binding = 2, std140) uniform Params {
  vec4 tint;
} params;

layout(set = 2, binding = 4, std430) readonly buffer Values {
  vec4 value;
} values;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main() {
  outColor = texture(textures[1], uv) * params.tint + values.value;
}
