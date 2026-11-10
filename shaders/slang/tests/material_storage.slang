struct TransformPushConstants {
  float4x4 mvp;
};

[[vk::push_constant]]
TransformPushConstants push_constants;

struct VertexInput {
  [[vk::location(0)]] float3 position : POSITION0;
  [[vk::location(1)]] float3 color : COLOR0;
};

struct VertexOutput
{
  float4 position : SV_Position;
  [[vk::location(0)]]
  float3 color : COLOR0;
};

struct MaterialParameters {
  float4 colorMultiplier;
};

[[vk::binding(2, 1)]]
StructuredBuffer<MaterialParameters> material;

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
  VertexOutput output;

  output.position = mul(push_constants.mvp, float4(input.position, 1.0));
  output.color = input.color * material[0].colorMultiplier.rgb;

  return output;
}
