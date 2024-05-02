#pragma once
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/UniquePtr.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion
{
    struct VulkanBindingValue : BindingValue
    {
        union
        {
            Texture     m_texture;
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
        RID                                            shader;
        BindingSetType                                 bindingSetType;

        VulkanBindingSet(const RID& shader, BindingSetType bindingSetType)
            : shader(shader),
              bindingSetType(bindingSetType)
        {
        }

        HashMap<String, UniquePtr<VulkanBindingValue>> bindingValues{};

        BindingValue& GetBindingValue(const StringView& name) override;
    };
}
