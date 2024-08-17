#include "ShaderAsset.hpp"

#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Graphics/ShaderManager.hpp"
#include "Fyrion/IO/FileSystem.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::ShaderAsset");
    }

    void ShaderAsset::Compile()
    {
        Array<u8>              bytes{};
        Array<ShaderStageInfo> tempStages{};

        RenderApiType renderApi = Graphics::GetRenderApi();
        String source = FileSystem::ReadFileAsString(GetHandler()->GetAbsolutePath());

        if (shaderType == ShaderAssetType::Graphics)
        {
            if (!ShaderManager::CompileShader(ShaderCreation{.asset = this, .source = source, .entryPoint = "MainVS", .shaderStage = ShaderStage::Vertex, .renderApi = renderApi}, bytes))
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

            if (!ShaderManager::CompileShader(ShaderCreation{.asset = this, .source = source, .entryPoint = "MainPS", .shaderStage = ShaderStage::Pixel, .renderApi = renderApi}, bytes))
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
                if (!ShaderManager::CompileShader(ShaderCreation{.asset = this, .source = source, .entryPoint = "MainGS", .shaderStage = ShaderStage::Geometry, .renderApi = renderApi}, bytes))
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
        else if (shaderType == ShaderAssetType::Compute)
        {
            if (!ShaderManager::CompileShader(ShaderCreation{
                                                 .asset = this,
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

        stages = Traits::Move(tempStages);

        shaderInfo = ShaderManager::ExtractShaderInfo(bytes, stages, renderApi);

        SaveBuffer(spriv, bytes.Data(), bytes.Size());
        GetHandler()->Save();

        for (PipelineState pipelineState : pipelineDependencies)
        {
            if (shaderType == ShaderAssetType::Graphics)
            {
                Graphics::CreateGraphicsPipelineState({
                    .shader = this,
                    .pipelineState = pipelineState
                });
            }
            else if (shaderType == ShaderAssetType::Compute)
            {
                Graphics::CreateComputePipelineState({
                    .shader = this,
                    .pipelineState = pipelineState
                });
            }
        }

        for (const auto it : shaderDependencies)
        {
            it.first->Compile();
        }

        for(const auto it: bindingSetDependencies)
        {
            it.first->Reload();
        }

       // logger.Debug("Shader {} compiled sucessfully", GetPath());
    }

    void ShaderAsset::AddPipelineDependency(PipelineState pipelineState)
    {
        pipelineDependencies.EmplaceBack(pipelineState);
    }

    void ShaderAsset::AddShaderDependency(ShaderAsset* shaderAsset)
    {
        shaderDependencies.Emplace(shaderAsset);
    }

    void ShaderAsset::AddBindingSetDependency(BindingSet* bindingSet)
    {
        bindingSetDependencies.Insert(bindingSet);
    }

    void ShaderAsset::RemoveBindingSetDependency(BindingSet* bindingSet)
    {
        bindingSetDependencies.Erase(bindingSet);
    }

    // StringView ShaderAsset::GetDisplayName() const
    // {
    //     if (shaderType == ShaderAssetType::Include)
    //     {
    //         return "Include Shader";
    //     }
    //
    //     if (shaderType == ShaderAssetType::Graphics)
    //     {
    //         return "Graphics Shader";
    //     }
    //
    //     if (shaderType == ShaderAssetType::Compute)
    //     {
    //         return "Compute Shader";
    //     }
    //
    //     if (shaderType == ShaderAssetType::Raytrace)
    //     {
    //         return "Raytrace Shader";
    //     }
    //     return "Shader";
    // }

    Span<ShaderStageInfo> ShaderAsset::GetStages() const
    {
        return stages;
    }

    Array<u8> ShaderAsset::GetBytes() const
    {
        return LoadBuffer(spriv);
    }

    bool ShaderAsset::IsCompiled() const
    {
        return (!stages.Empty() && HasBuffer(spriv)) || shaderType == ShaderAssetType::Include;
    }

    void ShaderAsset::RegisterType(NativeTypeHandler<ShaderAsset>& type)
    {
        type.Attribute<AssetMeta>(AssetMeta{.displayName = "Shader"});
        type.Constructor<ShaderAsset>();
        type.Field<&ShaderAsset::spriv>("spriv");
        type.Field<&ShaderAsset::stages>("stages");
        type.Field<&ShaderAsset::shaderInfo>("shaderInfo");
        type.Field<&ShaderAsset::shaderType>("shaderType");
    }
}
