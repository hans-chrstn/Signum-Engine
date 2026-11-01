struct TransformPushConstants {
  float4x4 mvp;
  float4x4 normal_matrix;
};

[[vk::push_constant]]
TransformPushConstants push_constants;

struct VertexInput {
  [[vk::location(0)]]
float3 position : POSITION0;
};

struct VertexOutput {
    float4 position : SV_Position;
};

[shader("vertex")]
VertexOutput vertexMain(VertexInput input) {
  VertexOutput output;
  output.position = mul(push_constants.mvp, float4(input.position, 1.0));
  return output;
}

[shader("fragment")]
float4 fragmentMain() : SV_Target {
  return float4(0.5, 0.5, 0.5, 1.0);
}
