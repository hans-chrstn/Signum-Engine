struct VertexOutput
{
  float4 position : SV_Position;
  // opacity is handled separately
  float3 color : COLOR0;
};

[shader("vertex")]
// returns a 4 floating point value and marks 
// the return value as the vertex's position
VertexOutput vertexMain(uint vertexID : SV_VulkanVertexID)
{
  VertexOutput output;
  if (vertexID == 0U)
  {
    // bottom-left
    output.position =  float4(-0.5, -0.5, 0.0, 1.0);
    output.color = float3(1.0, 0.0, 0.0);
    return output;
  } else if (vertexID == 1U)
  {
    // bottom-right
    output.position = float4(0.5, -0.5, 0.0, 1.0);
    output.color = float3(0.0, 1.0, 0.0);
    return output;
  } else 
  {
    // top
    output.position = float4(0.0, 0.5, 0.0, 1.0);
    output.color = float3(0.0, 0.0, 1.0);
    return output;
  }

}

[shader("fragment")]
// Outputs the vertex position and color.
float4 fragmentMain(float3 color : COLOR0) : SV_Target
{
  return float4(color, 1.0);
}
