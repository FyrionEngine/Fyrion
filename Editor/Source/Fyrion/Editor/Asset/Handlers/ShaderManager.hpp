#pragma once


#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    struct ShaderCreation
    {
        ShaderAsset*  asset{};
        StringView    source{};
        StringView    entryPoint{};
        ShaderStage   shaderStage{};
        RenderApiType renderApi{};
    };
}

namespace Fyrion::ShaderManager
{
    FY_API bool       CompileShader(const ShaderCreation& shaderCreation, Array<u8>& bytes);
    FY_API ShaderInfo ExtractShaderInfo(const Span<u8>& bytes, const Span<ShaderStageInfo>& stages, RenderApiType renderApi);
}
