struct DirectionalLightUniformData {
    float4 direction;
    float4 color_intensity;
};

[[vk::binding(0, 0)]]
Sampler2D day_texture;

[[vk::binding(1, 0)]]
ConstantBuffer<DirectionalLightUniformData> directional_light;

[[vk::binding(2, 0)]]
Sampler2D night_texture;

[shader("fragment")]
float4 fragmentMain(
    float3 normal : TEXCOORD1,
    float2 uv : TEXCOORD0) : SV_Target {
    float3 surface_normal = normalize(normal);

    float3 light_direction =
        normalize(directional_light.direction.xyz);

    float n_dot_l = dot(surface_normal, light_direction);

    float diffuse = max(0.0, n_dot_l);

    float3 light_color = directional_light.color_intensity.xyz;

    float light_intensity = directional_light.color_intensity.w;

    float ambient_intensity = 0.05;

    float3 illumination = float3(ambient_intensity, ambient_intensity, ambient_intensity) 
                          + (1.0 - ambient_intensity) * diffuse * light_color * light_intensity;

    float4 day_surface_color = day_texture.Sample(uv);

    float3 day_illuminated_color = day_surface_color.rgb * illumination;

    float4 night_surface_color = night_texture.Sample(uv);

    float night_factor = 1.0 - smoothstep(-0.1, 0.1, n_dot_l);

    float3 night_illuminated_color = night_surface_color.rgb * night_factor;

    float3 final_color = day_illuminated_color + night_illuminated_color;

    return float4(final_color, day_surface_color.a);
}
