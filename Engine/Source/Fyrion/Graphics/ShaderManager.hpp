#pragma once

#include "GraphicsTypes.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion::ShaderManager
{
    FY_API bool       CompileShader(const ShaderCreation& shaderCreation, Array<u8>& bytes);
    FY_API ShaderInfo ExtractShaderInfo(const Span<u8>& bytes, const Span<ShaderStageInfo>& stages, RenderApiType renderApi);
}
