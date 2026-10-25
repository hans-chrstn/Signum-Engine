[shader("vertex")]
float4 vertexMain(uint vertexID : SV_VertexID) : SV_Position
{
    float x = vertexID == 0 ? -0.5 : 0.5;
    float y = vertexID == 2 ? 0.5 : -0.5;

    return float4(x, y, 0.0, 1.0);
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    return float4(0.2, 0.4, 0.8, 1.0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
}

