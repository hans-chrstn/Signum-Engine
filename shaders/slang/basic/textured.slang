struct TransformPushConstants {
    float4x4 mvp;
};

[[vk::push_constant]]
TransformPushConstants push_constants;

[[vk::binding(0, 0)]]
Sampler2D texture_sampler;

struct VertexInput {
    [[vk::location(0)]] float3 position : POSITION0;
    [[vk::location(1)]] float3 color : COLOR0;
    [[vk::location(2)]] float2 uv : TEXCOORD0;
};

struct VertexOutput {
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
    VertexOutput output;

    output.position = mul(push_constants.mvp, float4(input.position, 1.0));
    output.uv = input.uv;

    return output;
}

[shader("fragment")]
float4 fragmentMain(float2 uv : TEXCOORD0) : SV_Target {
    return texture_sampler.Sample(uv);
}

