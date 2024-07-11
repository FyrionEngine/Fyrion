#include "VulkanBindingSet.hpp"
#include "Fyrion/Graphics/ShaderAsset.hpp"

namespace Fyrion
{
    VulkanBindingSet::VulkanBindingSet(ShaderAsset* shaderAsset, VulkanDevice& vulkanDevice, BindingSetType bindingSetType) : shaderAsset(shaderAsset),
                                                                                                                              vulkanDevice(vulkanDevice),
                                                                                                                              bindingSetType(bindingSetType)
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

    BindingValue* VulkanBindingSet::GetValue(const StringView& name)
    {
        auto it = bindingValues.Find(name);
        if (it == bindingValues.end())
        {
            it = bindingValues.Emplace(String{name}, MakeShared<VulkanBindingValue>()).first;
        }
        return it->second.Get();
    }
}
