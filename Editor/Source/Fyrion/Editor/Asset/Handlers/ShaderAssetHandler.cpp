#include "ShaderManager.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Editor/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"
#include "Fyrion/IO/FileSystem.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::ShaderAssetHandler", LogLevel::Debug);
    }

    class ShaderAssetHandler : public AssetHandler
    {
    public:
        FY_BASE_TYPES(AssetHandler);

        TypeID GetAssetTypeID() override
        {
            return GetTypeID<ShaderAsset>();
        }

        void Save(StringView newPath, AssetFile* assetFile) override {}

        virtual ShaderAssetType GetType() = 0;

        //TODO: report shader compilation error.

        void Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance) override
        {
            Array<u8>              bytes{};
            Array<ShaderStageInfo> tempStages{};

            ShaderAsset* shaderAsset = static_cast<ShaderAsset*>(instance);
            shaderAsset->type = GetType();

            RenderApiType   renderApi = Graphics::GetRenderApi();
            String          source = FileSystem::ReadFileAsString(assetFile->absolutePath);

            if (shaderAsset->type == ShaderAssetType::Graphics)
            {
                if (!ShaderManager::CompileShader(ShaderCreation{.asset = shaderAsset, .source = source, .entryPoint = "MainVS", .shaderStage = ShaderStage::Vertex, .renderApi = renderApi}, bytes))
                {
                    return;
                }

                tempStages.EmplaceBack(ShaderStageInfo{
                    .stage = ShaderStage::Vertex,
                    .entryPoint = "MainVS",
                    .offset = 0,
                    .size = (u32)bytes.Size()
                });

                u32 offset = (u32)bytes.Size();

                if (!ShaderManager::CompileShader(ShaderCreation{.asset = shaderAsset, .source = source, .entryPoint = "MainPS", .shaderStage = ShaderStage::Pixel, .renderApi = renderApi}, bytes))
                {
                    return;
                }

                tempStages.EmplaceBack(ShaderStageInfo{
                    .stage = ShaderStage::Pixel,
                    .entryPoint = "MainPS",
                    .offset = offset,
                    .size = (u32)bytes.Size() - offset
                });

                offset = (u32)bytes.Size();


                std::string_view str = {source.CStr(), source.Size()};
                if (str.find("MainGS") != std::string_view::npos)
                {
                    if (!ShaderManager::CompileShader(ShaderCreation{.asset = shaderAsset, .source = source, .entryPoint = "MainGS", .shaderStage = ShaderStage::Geometry, .renderApi = renderApi}, bytes))
                    {
                        return;
                    }

                    tempStages.EmplaceBack(ShaderStageInfo{
                        .stage = ShaderStage::Geometry,
                        .entryPoint = "MainGS",
                        .offset = offset,
                        .size = (u32)bytes.Size() - offset
                    });

                    offset = (u32)bytes.Size();
                }
            }
            else if (shaderAsset->type == ShaderAssetType::Compute)
            {
                if (!ShaderManager::CompileShader(ShaderCreation{
                                                      .asset = shaderAsset,
                                                      .source = source,
                                                      .entryPoint = "MainCS",
                                                      .shaderStage = ShaderStage::Compute,
                                                      .renderApi = renderApi
                                                  }, bytes))
                {
                    return;
                }

                tempStages.EmplaceBack(ShaderStageInfo{
                    .stage = ShaderStage::Compute,
                    .entryPoint = "MainCS",
                    .size = (u32)bytes.Size()
                });
            }

            shaderAsset->stages = Traits::Move(tempStages);
            shaderAsset->shaderInfo = ShaderManager::ExtractShaderInfo(bytes, shaderAsset->stages, renderApi);
            shaderAsset->bytes = bytes;


            for (PipelineState pipelineState : shaderAsset->pipelineDependencies)
            {
                if (shaderAsset->type == ShaderAssetType::Graphics)
                {
                    Graphics::CreateGraphicsPipelineState({
                        .shader = shaderAsset,
                        .pipelineState = pipelineState
                    });
                }
                else if (shaderAsset->type == ShaderAssetType::Compute)
                {
                    Graphics::CreateComputePipelineState({
                        .shader = shaderAsset,
                        .pipelineState = pipelineState
                    });
                }
            }

            for (const auto it : shaderAsset->shaderDependencies)
            {
                Assets::Reload(it.first->GetUUID());
            }

            for (const auto it : shaderAsset->bindingSetDependencies)
            {
                it.first->Reload();
            }

            logger.Debug("shader {} compiled sucessfully", assetFile->path);
        }

        void OpenAsset(AssetFile* assetFile) override
        {
            Assets::Load<ShaderAsset>(assetFile->uuid);
        }

        Image GenerateThumbnail(AssetFile* assetFile) override
        {
            return Image{};
        }
    };

    class ShaderIncludeHandler : public AssetHandler
    {
    public:
        FY_BASE_TYPES(AssetHandler);

        TypeID GetAssetTypeID() override
        {
            return GetTypeID<ShaderAsset>();
        }

        void Save(StringView newPath, AssetFile* assetFile) override {}
        void Load(AssetFile* assetFile, TypeHandler* typeHandler, VoidPtr instance) override {}
        void OpenAsset(AssetFile* assetFile) override {}

        Image GenerateThumbnail(AssetFile* assetFile) override
        {
            return {};
        }
    };

    class RasterShaderAssetHandler : public ShaderAssetHandler
    {
    public:
        FY_BASE_TYPES(ShaderAssetHandler);

        StringView Extension() override
        {
            return ".raster";
        }

        ShaderAssetType GetType() override
        {
            return ShaderAssetType::Graphics;
        }
    };

    class ComputeShaderAssetHandler : public ShaderAssetHandler
    {
    public:
        FY_BASE_TYPES(ShaderAssetHandler);

        StringView Extension() override
        {
            return ".comp";
        }

        ShaderAssetType GetType() override
        {
            return ShaderAssetType::Compute;
        }
    };


    class HLSLShaderIncludeHandler : public ShaderIncludeHandler
    {
    public:
        FY_BASE_TYPES(ShaderIncludeHandler);

        StringView Extension() override
        {
            return ".hlsl";
        }
    };

    class IncShaderIncludeHandler : public ShaderIncludeHandler
    {
    public:
        FY_BASE_TYPES(ShaderIncludeHandler);

        StringView Extension() override
        {
            return ".inc";
        }
    };


    void RegisterShaderAssetHandlers()
    {
        Registry::Type<ShaderAssetHandler>();
        Registry::Type<RasterShaderAssetHandler>();
        Registry::Type<ComputeShaderAssetHandler>();
        Registry::Type<ShaderIncludeHandler>();
        Registry::Type<HLSLShaderIncludeHandler>();
        Registry::Type<IncShaderIncludeHandler>();
    }
}
