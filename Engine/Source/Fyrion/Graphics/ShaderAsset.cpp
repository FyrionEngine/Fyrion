#include "ShaderAsset.hpp"

#include "Graphics.hpp"
#include "ShaderManager.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/IO/FileSystem.hpp"


namespace Fyrion
{
    class Logger;

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::ShaderAsset", LogLevel::Debug);
    }

    Span<StringView> RasterShaderIO::GetImportExtensions()
    {
        return {extension, 1};
    }

    Asset* RasterShaderIO::ImportAsset(StringView path, Asset* reimportAsset)
    {
        ShaderAsset* shaderAsset = AssetDatabase::Create<ShaderAsset>();
        shaderAsset->SetShaderSource(FileSystem::ReadFileAsString(path));
        shaderAsset->SetShaderType(ShaderAssetType::Raster);
        shaderAsset->Compile();
        return shaderAsset;
    }

    StringView ShaderAsset::GetDisplayName() const
    {
        switch (shaderType)
        {
            case ShaderAssetType::Raster: return "Raster Shader";
            case ShaderAssetType::Compute: return "Compute Shader";
            case ShaderAssetType::Raytrace: return "Raytrace Shader";
            default: return "None";
        }
    }

    void ShaderAsset::Compile()
    {
        RenderApiType renderApi = Graphics::GetRenderApi();

        StringView source = GetShaderSource();

        if (shaderType == ShaderAssetType::Raster)
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

        shaderInfo = ShaderManager::ExtractShaderInfo(bytes, stages, renderApi);

        logger.Debug("Shader {} compiled sucessfully", GetPath());
    }

    void ShaderAsset::RegisterType(NativeTypeHandler<ShaderAsset>& type)
    {
        type.Constructor<ShaderAsset>();
    }
}
