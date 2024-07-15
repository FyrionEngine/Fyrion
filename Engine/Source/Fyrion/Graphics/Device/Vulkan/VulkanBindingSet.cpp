#include "VulkanBindingSet.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"

namespace Fyrion
{
    VulkanBindingSet::VulkanBindingSet(ShaderAsset* shaderAsset, VulkanDevice& vulkanDevice) : vulkanDevice(vulkanDevice),
                                                                                               shaderAsset(shaderAsset)
    {
        const ShaderInfo& shaderInfo = shaderAsset->GetShaderInfo();

        for (const DescriptorLayout& descriptorLayout : shaderInfo.descriptors)
        {
            auto setIt = descriptorLayoutLookup.Find(descriptorLayout.set);

            if (setIt == descriptorLayoutLookup.end())
            {
                descriptorLayoutLookup.Insert(descriptorLayout.set, descriptorLayout);
            }

            for (const DescriptorBinding& binding : descriptorLayout.bindings)
            {
                if (auto it = valueDescriptorSetLookup.Find(binding.name); it == valueDescriptorSetLookup.end())
                {
                    valueDescriptorSetLookup.Emplace(String{binding.name}, (u32)descriptorLayout.set);
                }
            }
        }
    }

    void VulkanBindingVar::SetTexture(const Texture& texture)
    {
        if (texture != m_texture)
        {
            m_texture = texture;
        }
    }

    void VulkanBindingVar::SetTextureView(const TextureView& textureView)
    {
        m_textureView = textureView;
    }

    void VulkanBindingVar::SetSampler(const Sampler& sampler)
    {
        m_sampler = sampler;
    }

    void VulkanBindingVar::SetBuffer(const Buffer& buffer)
    {
        m_buffer = buffer;
    }

    void VulkanBindingVar::SetValue(ConstPtr ptr, usize size)
    {
        if (!m_buffer)
        {

        }
    }

    BindingVar* VulkanBindingSet::GetVar(const StringView& name)
    {
        auto it = bindingValues.Find(name);
        if (it == bindingValues.end())
        {
            it = bindingValues.Emplace(String{name}, MakeShared<VulkanBindingVar>(*this)).first;
        }
        return it->second.Get();
    }
}
