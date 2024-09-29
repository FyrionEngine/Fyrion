#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"


namespace Fyrion
{
    class FY_API ShaderAsset
    {
    public:
        void AddPipelineDependency(PipelineState pipelineState);
        void AddShaderDependency(ShaderAsset* shaderAsset);
        void AddBindingSetDependency(BindingSet* bindingSet);
        void RemoveBindingSetDependency(BindingSet* bindingSet);

        const ShaderInfo& GetShaderInfo() const;

        bool                  IsCompiled() const;
        Span<ShaderStageInfo> GetStages() const;
        Array<u8>             GetBytes() const;

    private:
        ShaderInfo             shaderInfo;
        Array<ShaderStageInfo> stages{};
    };
}
