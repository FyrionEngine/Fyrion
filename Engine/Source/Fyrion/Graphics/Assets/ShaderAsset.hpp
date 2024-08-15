#pragma once
#include "Fyrion/Asset/Asset.hpp"
#include "Fyrion/Core/HashSet.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

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

    class FY_API ShaderAsset : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        StringView GetShaderSource() const
        {
            return shaderSource;
        }

        ShaderAssetType GetShaderType() const
        {
            return shaderType;
        }

        void SetShaderSource(const String& shaderSource)
        {
            this->shaderSource = shaderSource;
        }

        void SetShaderType(ShaderAssetType shaderType)
        {
            this->shaderType = shaderType;
        }

        const ShaderInfo& GetShaderInfo() const
        {
            FY_ASSERT(IsCompiled(), "shader is not compiled");
            return shaderInfo;
        }

        Span<ShaderStageInfo> GetStages() const;
        Array<u8>             GetBytes() const;

        bool IsCompiled() const;
        void Compile();

        void AddPipelineDependency(PipelineState pipelineState);
        void AddShaderDependency(ShaderAsset* shaderAsset);
        void AddBindingSetDependency(BindingSet* bindingSet);
        void RemoveBindingSetDependency(BindingSet* bindingSet);

        static void RegisterType(NativeTypeHandler<ShaderAsset>& type);

    private:
        String                 shaderSource;
        ShaderAssetType        shaderType = ShaderAssetType::None;
        AssetBuffer            spriv{};
        Array<ShaderStageInfo> stages{};
        ShaderInfo             shaderInfo{};
        Array<PipelineState>   pipelineDependencies{};
        HashSet<ShaderAsset*>  shaderDependencies{};
        HashSet<BindingSet*>   bindingSetDependencies{};
    };
}
