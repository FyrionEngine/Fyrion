
struct HBAOConstantBuffer
{
    float4 fields;//x = inverseDepthRangeA; y = inverseDepthRangeB, z = linearizeDepthA, w = linearizeDepthB;

    float GetInverseDepthRangeA() { return fields.x; }
    float GetInverseDepthRangeB() { return fields.y; }
    float GetLinearizeDepthA() { return fields.z; }
    float GetLinearizeDepthB() { return fields.w; }
};