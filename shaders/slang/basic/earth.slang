struct DirectionalLightUniformData {
    float4 direction;
    float4 color_intensity;
};

struct CloudUniformData {
    float4 uv_offset;
};

static const float specular_shininess = 48.0;
static const float specular_strength = 0.35;

[[vk::binding(0, 0)]]
Sampler2D day_texture;

[[vk::binding(1, 0)]]
ConstantBuffer<DirectionalLightUniformData> directional_light;

[[vk::binding(2, 0)]]
Sampler2D night_texture;

[[vk::binding(3, 0)]]
Sampler2D cloud_texture;

[[vk::binding(4, 0)]]
ConstantBuffer<CloudUniformData> cloud_uniform;

[[vk::binding(5, 0)]]
Sampler2D specular_texture;

[[vk::binding(7, 0)]]
Sampler2D normal_texture;

[shader("fragment")]
float4 fragmentMain(
    float3 normal : TEXCOORD1,
    float2 uv : TEXCOORD0,
    float4 tangent : TEXCOORD2) : SV_Target {
    float3 surface_normal = normalize(normal);

    float3 tangent_direction = normalize(
        tangent.xyz -
        surface_normal * dot(surface_normal, tangent.xyz));

    float3 bitangent_direction =
        normalize(cross(surface_normal, tangent_direction)) * tangent.w;

    float3 tangent_space_normal =
        normal_texture.Sample(uv).xyz * 2.0 - 1.0;

    float3 mapped_normal = normalize(
        tangent_direction * tangent_space_normal.x +
        bitangent_direction * tangent_space_normal.y +
        surface_normal * tangent_space_normal.z);

    float3 light_direction =
        normalize(directional_light.direction.xyz);

    float n_dot_l = dot(mapped_normal, light_direction);

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

    float specular_mask = specular_texture.Sample(uv).r;

    float3 view_direction = float3(0.0, 0.0, 1.0);

    float3 half_direction =
        normalize(light_direction + view_direction);

    float specular_factor = pow(
        max(dot(mapped_normal, half_direction), 0.0),
        specular_shininess);

    float3 specular_color =
        specular_mask *
        specular_factor *
        diffuse *
        light_color *
        light_intensity *
        specular_strength;

    float2 cloud_uv = uv + cloud_uniform.uv_offset.xy;

    float cloud_coverage = cloud_texture.Sample(cloud_uv).r;

    float3 cloud_color = float3(1.0, 1.0, 1.0) * illumination;

    float3 illuminated_surface =
        day_illuminated_color +
        night_illuminated_color +
        specular_color;

    float3 final_color = lerp(
        illuminated_surface,
        cloud_color,
        cloud_coverage);

    return float4(final_color, day_surface_color.a);
}
