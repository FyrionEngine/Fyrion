#include "Fyrion://Shaders/Passes/HBAOPlus/SharedDefs.inc"

Texture2D<float2>     AODepthTexture     : register(t1);
sampler               PointClampSampler  : register(s2);
sampler               LinearClampSampler : register(s3);


struct BlurParameters
{
    float2 pos;
    float2 uv;
};


//----------------------------------------------------------------------------------
float2 PointSampleAODepth(float2 UV)
{
    return AODepthTexture.Sample(PointClampSampler, UV).xy;
}
float2 LinearSampleAODepth(float2 UV)
{
    return AODepthTexture.Sample(LinearClampSampler, UV).xy;
}

//----------------------------------------------------------------------------------
struct CenterPixelData
{
    float2 UV;
    float Depth;
    float Sharpness;
    float Scale;
    float Bias;
};

//----------------------------------------------------------------------------------
float CrossBilateralWeight(float R, float SampleDepth, float DepthSlope, CenterPixelData Center)
{
    const float BlurSigma = ((float)KERNEL_RADIUS+1.0) * 0.5;
    const float BlurFalloff = 1.0 / (2.0*BlurSigma*BlurSigma);

    SampleDepth -= DepthSlope * R;
    float DeltaZ = SampleDepth * Center.Scale + Center.Bias;
    return exp2(-R*R*BlurFalloff - DeltaZ*DeltaZ);
}

//-------------------------------------------------------------------------
void ProcessSample(float2 AOZ,
                   float R,
                   float DepthSlope,
                   CenterPixelData Center,
                   inout float TotalAO,
                   inout float TotalW)
{
    float AO = AOZ.x;
    float Z = AOZ.y;

    float W = CrossBilateralWeight(R, Z, DepthSlope, Center);
    TotalAO += W * AO;
    TotalW += W;
}

//-------------------------------------------------------------------------
void ProcessRadius(float R0,
                   float2 DeltaUV,
                   float DepthSlope,
                   CenterPixelData Center,
                   inout float TotalAO,
                   inout float TotalW)
{
    float R = R0;

    [unroll]
    for (; R <= KERNEL_RADIUS/2; R += 1)
    {
        float2 UV = R * DeltaUV + Center.UV;
        float2 AOZ = PointSampleAODepth(UV);
        ProcessSample(AOZ, R, DepthSlope, Center, TotalAO, TotalW);
    }

    [unroll]
    for (; R <= KERNEL_RADIUS; R += 2)
    {
        float2 UV = (R + 0.5) * DeltaUV + Center.UV;
        float2 AOZ = LinearSampleAODepth(UV);
        ProcessSample(AOZ, R, DepthSlope, Center, TotalAO, TotalW);
    }
}

//-------------------------------------------------------------------------
void ProcessRadius1(float2 DeltaUV,
                    CenterPixelData Center,
                    inout float TotalAO,
                    inout float TotalW)
{
    float2 AODepth = PointSampleAODepth(Center.UV + DeltaUV);
    float DepthSlope = AODepth.y - Center.Depth;

    ProcessSample(AODepth, 1, DepthSlope, Center, TotalAO, TotalW);
    ProcessRadius(2, DeltaUV, DepthSlope, Center, TotalAO, TotalW);
}

//-------------------------------------------------------------------------
float GetSharpness(float ViewDepth)
{
    float lerpFactor = (ViewDepth - hbao.blurViewDepth0) / (hbao.blurViewDepth1 - hbao.blurViewDepth0);
    return lerp(hbao.blurSharpness0, hbao.blurSharpness1, saturate(lerpFactor));
}

//-------------------------------------------------------------------------
float ComputeBlur(BlurParameters params,
                  float2 DeltaUV,
                  out float CenterDepth)
{
    float2 AOZ = PointSampleAODepth(params.uv);
    CenterDepth = AOZ.y;

    CenterPixelData Center;
    Center.UV = params.uv;
    Center.Depth = CenterDepth;
    Center.Sharpness = GetSharpness(CenterDepth);
    Center.Scale = Center.Sharpness;
    Center.Bias = -Center.Depth * Center.Sharpness;

    float TotalAO = AOZ.x;
    float TotalW = 1.0;

    ProcessRadius1(DeltaUV, Center, TotalAO, TotalW);
    ProcessRadius1(-DeltaUV, Center, TotalAO, TotalW);

    return TotalAO / TotalW;
}
