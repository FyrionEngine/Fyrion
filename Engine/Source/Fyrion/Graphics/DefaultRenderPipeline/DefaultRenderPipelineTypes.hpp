#pragma once
#include "Fyrion/Core/Math.hpp"

#define FY_SHADOW_MAP_CASCADE_COUNT 4
#define FY_SHADOW_MAP_DIM 4096

namespace Fyrion
{
    struct ShadowMapDataInfo
    {
        f32  cascadeSplit[FY_SHADOW_MAP_CASCADE_COUNT];
        Mat4 cascadeViewProjMat[FY_SHADOW_MAP_CASCADE_COUNT];
    };
}
