struct DirectionalLightUniformData {
    float4 direction;
    float4 color_intensity;
};

[[vk::binding(0, 0)]]
Sampler2D surface_texture;

[[vk::binding(1, 0)]]
ConstantBuffer<DirectionalLightUniformData> directional_light;

[[vk::binding(2, 0)]]
Sampler2D detail_texture;

[shader("fragment")]
float4 fragmentMain(
    float3 normal : TEXCOORD1,
    float2 uv : TEXCOORD0) : SV_Target {

    float4 surface_color = surface_texture.Sample(uv);
    float4 detail_color = detail_texture.Sample(uv);

    float3 combined_color =
        lerp(surface_color.rgb, detail_color.rgb, 0.5);

    float3 surface_normal = normalize(normal);
    float3 light_direction = normalize(directional_light.direction.xyz);

    float diffuse =
        max(0.0, dot(surface_normal, light_direction));

    float3 light_color =
        directional_light.color_intensity.xyz;

    float light_intensity =
        directional_light.color_intensity.w;

    float3 illumination =
        float3(0.05, 0.05, 0.05) +
        0.95 * diffuse * light_color * light_intensity;

    return float4(combined_color * illumination, 1.0);
}

