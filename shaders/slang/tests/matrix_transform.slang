struct ModelPushConstants {
  float4x4 model;
};

[[vk::push_constant]]
ModelPushConstants push_constants;

[[vk::binding(0, 0)]] // descriptor binding 0
RWStructuredBuffer<float4> output_buffer; // store gpu calculated result

[shader("compute")]
[numthreads(1, 1, 1)] // one thread per workgroup
void computeMain()
{
    static const float4 kTestPosition = float4(1.0, 2.0, 0.0, 1.0);
    static const uint kOutputIndex = 0;

    const float4 transformed_position =
        mul(push_constants.model, kTestPosition);

    output_buffer[kOutputIndex] = transformed_position;
}
