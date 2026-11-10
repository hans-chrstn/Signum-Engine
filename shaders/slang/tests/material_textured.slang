struct TransformPushConstants {
  float4x4 mvp;
};

[[vk::push_constant]]
TransformPushConstants push_constants;

struct VertexInput {
  [[vk::location(0)]] float3 position : POSITION0;
  [[vk::location(1)]] float3 normal : NORMAL0;
  [[vk::location(2)]] float2 uv : TEXCOORD0;
};

struct VertexOutput
{
  float4 position : SV_Position;
  [[vk::location(0)]]
  float3 normal : TEXCOORD0;
  [[vk::location(1)]]
  float2 uv : TEXCOORD0;
};

struct MaterialParameters {
  float4 uvOffset;
};

[[vk::binding(2, 1)]]
ConstantBuffer<MaterialParameters> material;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
  VertexOutput output;

  output.position = mul(push_constants.mvp, float4(input.position, 1.0));
  output.normal = input.normal;
  output.uv = input.uv + material.uvOffset.xy * 0.01;

  return output;
}
