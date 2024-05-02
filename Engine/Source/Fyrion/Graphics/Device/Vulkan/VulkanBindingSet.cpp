#include "VulkanBindingSet.hpp"

namespace Fyrion
{
    void VulkanBindingValue::SetTexture(const Texture& texture)
    {
        if (texture != m_texture)
        {
            m_texture = texture;
        }
    }

    void VulkanBindingValue::SetTextureView(const TextureView& textureView)
    {
        m_textureView = textureView;
    }

    void VulkanBindingValue::SetSampler(const Sampler& sampler)
    {
        m_sampler = sampler;
    }

    void VulkanBindingValue::SetBuffer(const Buffer& buffer)
    {
        m_buffer = buffer;
    }

    BindingValue& VulkanBindingSet::GetBindingValue(const StringView& name)
    {
        auto it = bindingValues.Find(name);
        if (it == bindingValues.end())
        {
            it = bindingValues.Emplace(String{name}, MakeUnique<VulkanBindingValue>()).first;
        }
        return *it->second;
    }
}
