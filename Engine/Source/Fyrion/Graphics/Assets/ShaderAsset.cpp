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

        Array<u8>              tempBytes{};
        Array<ShaderStageInfo> tempStages{};

        RenderApiType renderApi = Graphics::GetRenderApi();
        StringView    source = GetShaderSource();

        if (shaderType == ShaderAssetType::Graphics)
        {
            if (!ShaderManager::CompileShader(ShaderCreation{.asset = this, .source = source, .entryPoint = "MainVS", .shaderStage = ShaderStage::Vertex, .renderApi = renderApi}, tempBytes))
            {
                return;
            }

            tempStages.EmplaceBack(ShaderStageInfo{
                .stage = ShaderStage::Vertex,
                .entryPoint = "MainVS",
                .offset = 0,
                .size = (u32)tempBytes.Size()
            });

            u32 pixelOffset = (u32)tempBytes.Size();

            if (!ShaderManager::CompileShader(ShaderCreation{.asset = this, .source = source, .entryPoint = "MainPS", .shaderStage = ShaderStage::Pixel, .renderApi = renderApi}, tempBytes))
            {
                return;
            }

            tempStages.EmplaceBack(ShaderStageInfo{
                .stage = ShaderStage::Pixel,
                .entryPoint = "MainPS",
                .offset = pixelOffset,
                .size = (u32)tempBytes.Size() - pixelOffset
            });
        }
        else if (shaderType == ShaderAssetType::Compute)
        {
            if (!ShaderManager::CompileShader(ShaderCreation{
                                                 .asset = this,
                                                 .source = source,
                                                 .entryPoint = "MainCS",
                                                 .shaderStage = ShaderStage::Compute,
                                                 .renderApi = renderApi
                                             }, tempBytes))
            {
                return;
            }

            tempStages.EmplaceBack(ShaderStageInfo{
                .stage = ShaderStage::Compute,
                .entryPoint = "MainCS",
                .size = (u32)tempBytes.Size()
            });

        }

        stages = Traits::Move(tempStages);
        bytes = Traits::Move(tempBytes);

        shaderInfo = ShaderManager::ExtractShaderInfo(bytes, stages, renderApi);

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

        logger.Debug("Shader {} compiled sucessfully", GetPath());
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

    StringView ShaderAsset::GetDisplayName() const
    {
        if (shaderType == ShaderAssetType::Include)
        {
            return "Include Shader";
        }

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

    bool ShaderAsset::IsCompiled() const
    {
        return (!stages.Empty() && !bytes.Empty()) || shaderType == ShaderAssetType::Include;
    }

    void ShaderAsset::RegisterType(NativeTypeHandler<ShaderAsset>& type)
    {
        type.Constructor<ShaderAsset>();
        type.Field<&ShaderAsset::shaderType>("shaderType");
    }
}
