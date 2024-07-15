#include "ShaderAsset.hpp"

#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/ShaderManager.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::ShaderAsset");
    }

    void ShaderAsset::Compile()
    {
        RenderApiType renderApi = Graphics::GetRenderApi();
        StringView    source = GetShaderSource();

        if (shaderType == ShaderAssetType::Graphics)
        {
            if (!ShaderManager::CompileShader(ShaderCreation{.source = source, .entryPoint = "MainVS", .shaderStage = ShaderStage::Vertex, .renderApi = renderApi}, bytes))
            {
                return;
            }

            stages.EmplaceBack(ShaderStageInfo{
                .stage = ShaderStage::Vertex,
                .entryPoint = "MainVS",
                .offset = 0,
                .size = (u32)bytes.Size()
            });

            u32 pixelOffset = (u32)bytes.Size();

            if (!ShaderManager::CompileShader(ShaderCreation{.source = source, .entryPoint = "MainPS", .shaderStage = ShaderStage::Pixel, .renderApi = renderApi}, bytes))
            {
                return;
            }

            stages.EmplaceBack(ShaderStageInfo{
                .stage = ShaderStage::Pixel,
                .entryPoint = "MainPS",
                .offset = pixelOffset,
                .size = (u32)bytes.Size() - pixelOffset
            });
        }
        else if (shaderType == ShaderAssetType::Compute)
        {
            if (ShaderManager::CompileShader(ShaderCreation{
                                                 .source = source,
                                                 .entryPoint = "MainCS",
                                                 .shaderStage = ShaderStage::Compute,
                                                 .renderApi = renderApi
                                             }, bytes))
            {
                stages.EmplaceBack(ShaderStageInfo{
                    .stage = ShaderStage::Compute,
                    .entryPoint = "MainCS",
                    .size = (u32)bytes.Size()
                });
            }
        }

        shaderInfo = ShaderManager::ExtractShaderInfo(bytes, stages, renderApi);
        logger.Debug("Shader {} compiled sucessfully", GetPath());
    }

    StringView ShaderAsset::GetDisplayName() const
    {
        if (shaderType == ShaderAssetType::Graphics)
        {
            return "Graphics Shader";
        }

        if (shaderType == ShaderAssetType::Compute)
        {
            return "Compute Shader";
        }

        if (shaderType == ShaderAssetType::Raytrace)
        {
            return "Raytrace Shader";
        }
        return "Shader";
    }

    Span<ShaderStageInfo> ShaderAsset::GetStages() const
    {
        return stages;
    }

    Span<u8> ShaderAsset::GetBytes() const
    {
        return bytes;
    }

    void ShaderAsset::RegisterType(NativeTypeHandler<ShaderAsset>& type)
    {
        type.Constructor<ShaderAsset>();
        type.Field<&ShaderAsset::shaderType>("shaderType");
    }
}
