[[vk::binding(0, 0)]]
Sampler2D materialTexture;

[shader("fragment")]
float4 fragmentMain(
    [[vk::location(1)]] float2 inUv : TEXCOORD1
) : SV_Target {
  return materialTexture.Sample(inUv);
}
