struct TransformPushConstants {
    float4x4 mvp;
    float4x4 normal_matrix;
};

struct DirectionalLightUniformData {
    float4 direction;
    float4 color_intensity;
};

[[vk::push_constant]]
TransformPushConstants push_constants;

[[vk::binding(0, 0)]]
Sampler2D texture_sampler;

[[vk::binding(1, 0)]]
ConstantBuffer<DirectionalLightUniformData> directional_light;

struct VertexInput {
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float3 normal : NORMAL0;
    [[vk::location(2)]] float2 uv : TEXCOORD0;
    [[vk::location(3)]] float4 tangent : TANGENT0;
};

struct VertexOutput {
    float4 position : SV_Position;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD0;
    float4 tangent : TEXCOORD2;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;

    output.position = mul(push_constants.mvp, float4(input.position, 1.0));
    output.normal = mul(push_constants.normal_matrix, float4(input.normal, 0.0)).xyz;
    output.uv = input.uv;
    output.tangent = float4(
        mul(push_constants.normal_matrix, float4(input.tangent.xyz, 0.0)).xyz,
        input.tangent.w
    );

    return output;
}

[shader("fragment")]
float4 fragmentMain(float3 normal : TEXCOORD1, float2 uv : TEXCOORD0) : SV_Target {
    float3 surface_normal = normalize(normal);
    float3 light_direction = directional_light.direction.xyz;
    float diffuse = max(0.0, dot(surface_normal, light_direction));
    float3 light_color = directional_light.color_intensity.xyz;
    float light_intensity = directional_light.color_intensity.w;
    float ambient_intensity = 0.05;
    float3 illumination = float3(ambient_intensity, ambient_intensity, ambient_intensity) 
                          + (1.0 - ambient_intensity) * diffuse * light_color * light_intensity;
    float4 surface_color = texture_sampler.Sample(uv);
    float3 illuminated_color = surface_color.rgb * illumination;

    return float4(illuminated_color, surface_color.a);
}

