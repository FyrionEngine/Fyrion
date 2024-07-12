#include "GraphicsAssets.hpp"

#include "Graphics.hpp"
#include "ShaderManager.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"


namespace Fyrion
{
    class Logger;

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::ShaderAsset", LogLevel::Debug);
    }

    Span<StringView> ShaderIO::GetImportExtensions()
    {
        return {extension, 2};
    }

    Asset* ShaderIO::CreateAsset()
    {
        return AssetDatabase::Create<ShaderAsset>();
    }

    void ShaderIO::ImportAsset(StringView path, Asset* asset)
    {
        ShaderAsset* shaderAsset = asset->Cast<ShaderAsset>();
        shaderAsset->SetShaderSource(FileSystem::ReadFileAsString(path));

        if (Path::Extension(path) == ".raster")
        {
            shaderAsset->SetShaderType(ShaderAssetType::Graphics);
        }
        else if (Path::Extension(path) == ".comp")
        {
            shaderAsset->SetShaderType(ShaderAssetType::Compute);
        }
        shaderAsset->Compile();
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

    void ShaderAsset::Compile()
    {
        RenderApiType renderApi = Graphics::GetRenderApi();
        StringView source = GetShaderSource();

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

    void ShaderAsset::RegisterType(NativeTypeHandler<ShaderAsset>& type)
    {
        type.Constructor<ShaderAsset>();
        type.Field<&ShaderAsset::shaderType>("shaderType");
    }

    Span<StringView> TextureIO::GetImportExtensions()
    {
        return {extension, 5};
    }

    Asset* TextureIO::CreateAsset()
    {
        return AssetDatabase::Create<TextureAsset>();
    }

    void TextureIO::ImportAsset(StringView path, Asset* asset)
    {
        TextureAsset* textureAsset = asset->Cast<TextureAsset>();
        textureAsset->SetImage(path);
    }

    StringView TextureAsset::GetDisplayName() const
    {
        return "Texture";
    }

    void TextureAsset::SetImage(StringView path)
    {
        Image image(path);
        Span<u8> imgData = image.GetData();
        data.Save(imgData.Data(), imgData.Size());
    }

    void TextureAsset::RegisterType(NativeTypeHandler<TextureAsset>& type)
    {
        type.Field<&TextureAsset::data>("data");
    }
}
