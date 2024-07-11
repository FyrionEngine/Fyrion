#pragma once
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    struct VulkanBindingSet;
    class VulkanDevice;

    struct VulkanBindingVar : BindingVar
    {
        VulkanBindingSet& bindingSet;

        VulkanBindingVar(VulkanBindingSet& bindingSet) : bindingSet(bindingSet) {}

        union
        {
            Texture     m_texture{};
            TextureView m_textureView;
            Sampler     m_sampler;
            Buffer      m_buffer;
        };

        void SetTexture(const Texture& texture) override;
        void SetTextureView(const TextureView& textureView) override;
        void SetSampler(const Sampler& sampler) override;
        void SetBuffer(const Buffer& buffer) override;
        void SetValue(ConstPtr ptr, usize size) override;
    };

    struct VulkanBindingSet : BindingSet
    {
        VulkanDevice&  vulkanDevice;
        ShaderAsset*   shaderAsset;

        HashMap<String, SharedPtr<VulkanBindingVar>> bindingValues;
        HashMap<String, u32>                         valueDescriptorSetLookup{};
        HashMap<u32, DescriptorLayout>               descriptorLayoutLookup{};


        VulkanBindingSet(ShaderAsset* shaderAsset, VulkanDevice& vulkanDevice);

        BindingVar* GetVar(const StringView& name) override;
    };
}
