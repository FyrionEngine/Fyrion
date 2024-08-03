static const float PI = 3.141592;
static const float TwoPI = 2 * PI;
static const float Epsilon = 0.001;
static const float minGGXAlpha = 0.0064f;

float3 GetSamplingVector(float outputWidth, float outputHeight, float outputDepth, uint3 threadID)
{
    float2 st = threadID.xy/float2(outputWidth, outputHeight);
    float2 uv = 2.0 * float2(st.x, 1.0-st.y) - 1.0;

    // Select vector based on cubemap face index.
    float3 ret;
    switch(threadID.z)
    {
        case 0: ret = float3(1.0,  uv.y, -uv.x); break;
        case 1: ret = float3(-1.0, uv.y,  uv.x); break;
        case 2: ret = float3(uv.x, 1.0, -uv.y); break;
        case 3: ret = float3(uv.x, -1.0, uv.y); break;
        case 4: ret = float3(uv.x, uv.y, 1.0); break;
        case 5: ret = float3(-uv.x, uv.y, -1.0); break;
    }
    return normalize(ret);
}


