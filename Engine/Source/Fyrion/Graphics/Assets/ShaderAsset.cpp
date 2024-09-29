#include "ShaderAsset.hpp"


namespace Fyrion
{
    void ShaderAsset::AddPipelineDependency(PipelineState pipelineState) {}
    void ShaderAsset::AddShaderDependency(ShaderAsset* shaderAsset) {}
    void ShaderAsset::AddBindingSetDependency(BindingSet* bindingSet) {}
    void ShaderAsset::RemoveBindingSetDependency(BindingSet* bindingSet) {}

    const ShaderInfo& ShaderAsset::GetShaderInfo() const
    {
        return shaderInfo;
    }

    bool ShaderAsset::IsCompiled() const
    {
        return false;
    }

    Span<ShaderStageInfo> ShaderAsset::GetStages() const
    {
        return stages;
    }

    Array<u8> ShaderAsset::GetBytes() const
    {
        return {};
    }
}
