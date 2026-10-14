#version 450

layout(set = 0, binding = 0) uniform texture2D sampled_image;
layout(set = 0, binding = 1) uniform sampler image_sampler;

layout(location = 0) in vec2 uv;
layout(location = 0) out vec4 outColor;

void main() {
  outColor = texture(sampler2D(sampled_image, image_sampler), uv);
}
