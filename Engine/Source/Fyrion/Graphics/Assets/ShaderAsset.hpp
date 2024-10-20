#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/IO/Asset.hpp"


namespace Fyrion
{
    enum class ShaderAssetType
    {
        None,
        Include,
        Graphics,
        Compute,
        Raytrace
    };

    struct FY_API ShaderAsset : public Asset
    {
        FY_BASE_TYPES(Asset);

        void AddPipelineDependency(PipelineState pipelineState);
        void AddShaderDependency(ShaderAsset* shaderAsset);
        void AddBindingSetDependency(BindingSet* bindingSet);
        void RemoveBindingSetDependency(BindingSet* bindingSet);

        ShaderInfo             shaderInfo;
        Array<ShaderStageInfo> stages{};
        Array<u8>              bytes{};
        ShaderAssetType        type = ShaderAssetType::None;

        Array<PipelineState>  pipelineDependencies{};
        HashSet<ShaderAsset*> shaderDependencies{};
        HashSet<BindingSet*>  bindingSetDependencies{};
    };
}
