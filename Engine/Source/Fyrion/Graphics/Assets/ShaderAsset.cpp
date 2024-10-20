#include "ShaderAsset.hpp"


namespace Fyrion
{
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
}
