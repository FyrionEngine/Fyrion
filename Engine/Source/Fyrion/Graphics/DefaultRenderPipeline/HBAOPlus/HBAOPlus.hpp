#pragma once
#include "Fyrion/Common.hpp"


namespace Fyrion::HBAOPlus
{
    enum class BlurRadius
    {
        BlurRadius2, // Kernel radius = 2 pixels
        BlurRadius4, // Kernel radius = 4 pixels (recommended)
    };

    enum class StepCount
    {
        StepCount4, // Use 4 steps per sampled direction (same as in HBAO+ 3.x)
        StepCount8, // Use 8 steps per sampled direction (slower, to reduce banding artifacts)
    };

    enum class DepthClampMode
    {
        ClampToEdge,   // Use clamp-to-edge when sampling depth (may cause false occlusion near screen borders)
        ClampToBorder, // Use clamp-to-border when sampling depth (may cause halos near screen borders)
    };

    struct ForegroundAO
    {
        bool enable = false;             // Enabling this may have a small performance impact
        f32  foregroundViewDepth = 0.0f; // View-space depth at which the AO footprint should get clamped
    };

    struct BackgroundAO
    {
        bool enable = false;             // Enabling this may have a small performance impact
        f32  backgroundViewDepth = 0.0f; // View-space depth at which the AO footprint should stop falling off with depth
    };

    struct DepthThreshold
    {
        bool enable = false;      // To return white AO for ViewDepths > MaxViewDepth
        f32  maxViewDepth = 0.0f; // Custom view-depth threshold
        f32  sharpness = 100.f;   // The higher, the sharper are the AO-to-white transitions
    };

    //---------------------------------------------------------------------------------------------------
    // When enabled, the actual per-pixel blur sharpness value depends on the per-pixel view depth with:
    //     LerpFactor = (PixelViewDepth - ForegroundViewDepth) / (BackgroundViewDepth - ForegroundViewDepth)
    //     Sharpness = lerp(Sharpness*ForegroundSharpnessScale, Sharpness, saturate(LerpFactor))
    //---------------------------------------------------------------------------------------------------
    struct BlurSharpnessProfile
    {
        bool enable = false;                 // To make the blur sharper in the foreground
        f32  foregroundSharpnessScale = 4.f; // Sharpness scale factor for ViewDepths <= ForegroundViewDepth
        f32  foregroundViewDepth = 0.f;      // Maximum view depth of the foreground depth range
        f32  backgroundViewDepth = 1.f;      // Minimum view depth of the background depth range
    };

    struct BlurParameters
    {
        bool                 enable = true;                    // To blur the AO with an edge-preserving blur
        BlurRadius           radius = BlurRadius::BlurRadius4; // BLUR_RADIUS_2 or BLUR_RADIUS_4
        f32                  sharpness = 16.f;                 // The higher, the more the blur preserves edges // 0.0~16.0
        BlurSharpnessProfile sharpnessProfile;                 // Optional depth-dependent sharpness function
    };

    struct HBAOPlusParameters
    {
        f32            radius = 1.f;                                 // The AO radius in meters
        f32            bias = 0.1f;                                  // To hide low-tessellation artifacts // 0.0~0.5
        f32            smallScaleAO = 1.f;                           // Scale factor for the small-scale AO, the greater the darker // 0.0~2.0
        f32            largeScaleAO = 1.f;                           // Scale factor for the large-scale AO, the greater the darker // 0.0~2.0
        f32            powerExponent = 2.f;                          // The final AO output is pow(AO, powerExponent) // 1.0~4.0
        ForegroundAO   foregroundAO;                                 // To limit the occlusion scale in the foreground
        BackgroundAO   backgroundAO;                                 // To add larger-scale occlusion in the distance
        StepCount      stepCount = StepCount::StepCount4;            // The number of steps per direction in the AO-generation pass
        DepthClampMode depthClampMode = DepthClampMode::ClampToEdge; // To hide possible false-occlusion artifacts near screen borders
        DepthThreshold depthThreshold;                               // Optional Z threshold, to hide possible depth-precision artifacts
        BlurParameters blur;                                         // Optional AO blur, to blur the AO before compositing it
        bool           enableDualLayerAO = true;                     // To reduce halo artifacts behind foreground object
    };
}
