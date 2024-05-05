#include "AssetTypes.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/Graphics/ShaderManager.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/Resource/ResourceAssets.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{
    struct ShaderAsset;

    RID ImportRasterShader(RID asset, const StringView& path)
    {
        RenderApiType renderApi = Graphics::GetRenderApi();

        String source = FileSystem::ReadFileAsString(path);

        Array<u8> bytes;
        Array<ShaderStageInfo> stages;

        bool vertexRes = ShaderManager::CompileShader(ShaderCreation{
                                                          .source = source,
                                                          .entryPoint = "MainVS",
                                                          .shaderStage = ShaderStage::Vertex,
                                                          .renderApi = renderApi
                                                      }, bytes);

        stages.EmplaceBack(ShaderStageInfo{
            .stage = ShaderStage::Vertex,
            .entryPoint = "MainVS",
            .offset = 0,
            .size = (u32)bytes.Size()
        });

        u32 pixelOffset = (u32) bytes.Size();

        bool pixelRes = ShaderManager::CompileShader(ShaderCreation{
                                                          .source = source,
                                                          .entryPoint = "MainPS",
                                                          .shaderStage = ShaderStage::Pixel,
                                                          .renderApi = renderApi
                                                      }, bytes);

        stages.EmplaceBack(ShaderStageInfo{
            .stage = ShaderStage::Pixel,
            .entryPoint = "MainPS",
            .offset = pixelOffset,
            .size = (u32)bytes.Size() - pixelOffset
        });

        if (vertexRes &&  pixelRes)
        {
            RID shader = Repository::CreateResource<ShaderAsset>();

            ResourceObject write = Repository::Write(shader);
            write[ShaderAsset::bytes] = bytes;
            write[ShaderAsset::info] = ShaderManager::ExtractShaderInfo(bytes, stages, renderApi);
            write[ShaderAsset::stages] = stages;

            write.Commit();

            return shader;
        }
        return {};
    }

    RID ImportCompShader(RID asset, const StringView& path)
    {
        RenderApiType renderApi = Graphics::GetRenderApi();

        String source = FileSystem::ReadFileAsString(path);

        Array<u8> bytes;

        if (ShaderManager::CompileShader(ShaderCreation{
                                             .source = source,
                                             .entryPoint = "MainCS",
                                             .shaderStage = ShaderStage::Compute,
                                             .renderApi = renderApi
                                         }, bytes))
        {
            Array<ShaderStageInfo> stages;

            RID shader = Repository::CreateResource<ShaderAsset>();

            stages.EmplaceBack(ShaderStageInfo{
                .stage = ShaderStage::Compute,
                .entryPoint = "MainCS",
                .size = (u32)bytes.Size()
            });

            ResourceObject write = Repository::Write(shader);
            write[ShaderAsset::bytes] = bytes;
            write[ShaderAsset::info] = ShaderManager::ExtractShaderInfo(bytes, stages, renderApi);
            write[ShaderAsset::stages] = stages;
            write.Commit();

            return shader;
        }

        return {};
    }

    void RegisterShaderAsset()
    {
        ResourceTypeBuilder<ShaderAsset>::Builder()
            .Value<ShaderAsset::bytes, Array<u8>>("Bytes")
            .Value<ShaderAsset::info, ShaderInfo>("Info")
            .Value<ShaderAsset::stages, Array<ShaderStageInfo>>("Stages")
            .Build();

        ResourceAssets::AddAssetImporter(".raster", ImportRasterShader);
        ResourceAssets::AddAssetImporter(".comp", ImportCompShader);
    }
}
