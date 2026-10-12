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
  // opacity is handled separately
  float3 color : COLOR0;
};

[shader("vertex")]
// returns a 4 floating point value and marks 
// the return value as the vertex's position
VertexOutput vertexMain(VertexInput input)
{
  VertexOutput output;

  output.position = mul(push_constants.mvp, float4(input.position, 1.0));
  output.color = input.color;

  return output;
}

[shader("fragment")]
// Outputs the interpolated RGB color with full opacity.
float4 fragmentMain(float3 color : COLOR0) : SV_Target
{
  return float4(color, 1.0);
}
