struct OccasionalParameters {
  float4 colorMultiplier;
};

[[vk::binding(3, 1)]]
ConstantBuffer<OccasionalParameters> occasional;

[shader("fragment")]
float4 fragmentMain(float3 inColor : TEXCOORD0) : SV_Target {
  return float4(inColor * occasional.colorMultiplier.rgb, 1.0);
}

