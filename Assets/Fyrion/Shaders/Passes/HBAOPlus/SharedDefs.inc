
// Number of sampled directions per pixel
#ifndef NUM_DIRECTIONS
#define NUM_DIRECTIONS 8
#endif

// To disable the per-pixel randomization
#ifndef USE_RANDOM_TEXTURE
#define USE_RANDOM_TEXTURE 1
#endif

// Width of the tiled random texture
#ifndef RANDOM_TEXTURE_WIDTH
#define RANDOM_TEXTURE_WIDTH 4
#endif

#ifndef MAX_NUM_MRTS
#define MAX_NUM_MRTS 8
#endif

#ifndef NUM_STEPS
#define NUM_STEPS 8
#endif

#ifndef KERNEL_RADIUS
#define KERNEL_RADIUS 4
#endif

struct HBAOConstantBuffer
{
    float2 invQuarterResolution;
    float2 invFullResolution;

    float2 uvToViewA;
    float2 uvToViewB;

    float radiusToScreen;
    float r2;
    float negInvR2;
    float nDotVBias;

    float smallScaleAOAmount;
    float largeScaleAOAmount;
    float powExponent;
    int unused;

    float blurViewDepth0;
    float blurViewDepth1;
    float blurSharpness0;
    float blurSharpness1;

    float linearizeDepthA;
    float linearizeDepthB;
    float inverseDepthRangeA;
    float inverseDepthRangeB;

    float2 inputViewportTopLeft;
    float viewDepthThresholdNegInv;
    float viewDepthThresholdSharpness;

    float backgroundAORadiusPixels;
    float foregroundAORadiusPixels;
    int   debugNormalComponent;
    float pad0;

    float4x4 normalMatrix;
    float normalDecodeScale;
    float normalDecodeBias;
    float2 pad1;
};

ConstantBuffer<HBAOConstantBuffer>  hbao : register(b0, space1);


struct HBAOPerPassConstantStruct
{
    float4  jitter;
    float2  offset;
    float   fSliceIndex;
    uint    uSliceIndex;
};