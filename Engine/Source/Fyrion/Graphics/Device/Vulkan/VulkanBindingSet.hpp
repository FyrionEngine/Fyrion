#pragma once
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    class VulkanDevice;

    struct VulkanBindingValue : BindingValue
    {
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
    };

    struct VulkanBindingSet : BindingSet
    {
        VulkanDevice&  vulkanDevice;
        RID            shader;
        BindingSetType bindingSetType;

        HashMap<String, SharedPtr<VulkanBindingValue>> bindingValues;
        HashMap<String, u32>                           valueDescriptorSetLookup{};
        HashMap<u32, DescriptorLayout>                 descriptorLayoutLookup{};


        VulkanBindingSet(VulkanDevice& vulkanDevice, const RID& shader, BindingSetType bindingSetType);


        BindingValue& GetBindingValue(const StringView& name) override;
    };
}
