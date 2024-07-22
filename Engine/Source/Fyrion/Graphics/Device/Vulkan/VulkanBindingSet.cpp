#include "VulkanBindingSet.hpp"

#include "VulkanDevice.hpp"
#include "VulkanUtils.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
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

    VulkanBindingSet::~VulkanBindingSet()
    {
        for (auto& descriptorIt : descriptorSets)
        {
            for (auto& data : descriptorIt.second->data)
            {
                vkDestroyDescriptorSetLayout(vulkanDevice.device, data.descriptorSetLayout, nullptr);
            }
        }
    }

    void VulkanBindingVar::SetTexture(const Texture& p_texture)
    {
        if (texture != p_texture.handler)
        {
            texture = static_cast<VulkanTexture*>(p_texture.handler);
        }
    }

    void VulkanBindingVar::SetTextureView(const TextureView& p_textureView)
    {
        if (textureView != p_textureView.handler)
        {
            textureView = static_cast<VulkanTextureView*>(p_textureView.handler);
        }
    }

    void VulkanBindingVar::SetSampler(const Sampler& p_sampler)
    {
        if (sampler != p_sampler.handler)
        {
            sampler = static_cast<VulkanSampler*>(p_sampler.handler);
        }
    }

    void VulkanBindingVar::SetBuffer(const Buffer& p_buffer)
    {
        if (buffer != p_buffer.handler)
        {
            buffer = static_cast<VulkanBuffer*>(p_buffer.handler);
        }
    }

    void VulkanBindingVar::SetValue(ConstPtr ptr, usize size)
    {
        if (!buffer)
        {
            //TODO needs to create the buffer.
        }
    }

    BindingVar* VulkanBindingSet::GetVar(const StringView& name)
    {
        auto it = bindingValues.Find(name);
        if (it == bindingValues.end())
        {
            //find the attribute descriptor set number
            auto varDescriptorSetIt = valueDescriptorSetLookup.Find(name);
            if (varDescriptorSetIt == valueDescriptorSetLookup.end())
            {
                FY_ASSERT(false, "binding set not found");
                return {};
            }

            u32 set = varDescriptorSetIt->second;

            auto descriptorSetIt = descriptorSets.Find(set);

            //if there is no VulkanDescriptorSet for the attribute, create one.
            if (descriptorSetIt == descriptorSets.end())
            {
                descriptorSetIt = descriptorSets.Emplace(set, MakeShared<VulkanDescriptorSet>()).first;
            }

            DescriptorLayout& descriptorLayout = descriptorLayoutLookup[set];

            VulkanDescriptorSet* vulkanDescriptorSet = descriptorSetIt->second.Get();


            //TODO check if that's a different frame? and if there is already
            //vulkanDescriptorSet->data[vulkanDescriptorSet->frame[vulkanDevice.currentFrame]];

            if (vulkanDescriptorSet->data.Empty())
            {
                VulkanDescriptorSetData& data = vulkanDescriptorSet->data.EmplaceBack();
                data.frame = vulkanDevice.currentFrame;

                bool hasRuntimeArray = false;
                Vulkan::CreateDescriptorSetLayout(vulkanDevice.device, descriptorLayout, &data.descriptorSetLayout, &hasRuntimeArray);

                VkDescriptorSetAllocateInfo allocInfo{};
                allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
                allocInfo.descriptorPool = vulkanDevice.descriptorPool;
                allocInfo.descriptorSetCount = 1;
                allocInfo.pSetLayouts = &data.descriptorSetLayout;

                VkDescriptorSetVariableDescriptorCountAllocateInfoEXT countInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT};

                u32 maxBinding = MaxBindlessResources - 1;
                countInfo.descriptorSetCount = 1;
                countInfo.pDescriptorCounts = &maxBinding;

                if (hasRuntimeArray && vulkanDevice.deviceFeatures.bindlessSupported)
                {
                    allocInfo.pNext = &countInfo;
                }

                VkResult result = vkAllocateDescriptorSets(vulkanDevice.device, &allocInfo, &data.descriptorSet);

                if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL)
                {
                    vulkanDevice.logger.Error("VK_ERROR_OUT_OF_POOL_MEMORY");
                }
                else if (result != VK_SUCCESS)
                {
                    vulkanDevice.logger.Error("Error on vkAllocateDescriptorSets");
                }

                vulkanDescriptorSet->bindingVars.Resize(descriptorLayout.bindings.Size());
                vulkanDescriptorSet->descriptorWrites.Resize(descriptorLayout.bindings.Size());
                vulkanDescriptorSet->descriptorImageInfos.Resize(descriptorLayout.bindings.Size());
                vulkanDescriptorSet->descriptorBufferInfos.Resize(descriptorLayout.bindings.Size());

                for (int i = 0; i < descriptorLayout.bindings.Size(); ++i)
                {
                    const DescriptorBinding& descriptorBinding = descriptorLayout.bindings[i];

                    VulkanBindingVar* bindingVar = bindingValues.Emplace(descriptorBinding.name, MakeShared<VulkanBindingVar>(*this)).first->second.Get();
                    bindingVar->binding = descriptorBinding.binding;
                    bindingVar->descriptorType = descriptorBinding.descriptorType;
                    bindingVar->size = descriptorBinding.size;
                    vulkanDescriptorSet->bindingVars[i] = bindingVar;
                }
            }

            it = bindingValues.Find(name);
        }
        return it->second.Get();
    }

    void VulkanBindingSet::Bind(VulkanCommands& cmd, const PipelineState& pipeline)
    {
        VulkanPipelineState* vulkanPipelineState = static_cast<VulkanPipelineState*>(pipeline.handler);

        for (auto& descriptorIt : descriptorSets)
        {
            VulkanDescriptorSet*     descriptorSet = descriptorIt.second.Get();
            VulkanDescriptorSetData& data = descriptorSet->data[descriptorIt.second->frames[vulkanDevice.currentFrame]];

            if (data.dirty)
            {
                for (int b = 0; b < descriptorSet->descriptorWrites.Size(); ++b)
                {
                    VkWriteDescriptorSet& writeDescriptorSet = descriptorSet->descriptorWrites[b];
                    VulkanBindingVar*     vulkanBindingVar = descriptorSet->bindingVars[b];

                    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                    writeDescriptorSet.dstSet = data.descriptorSet;
                    writeDescriptorSet.descriptorCount = 1;
                    writeDescriptorSet.descriptorType = Vulkan::CastDescriptorType(vulkanBindingVar->descriptorType);
                    writeDescriptorSet.dstBinding = vulkanBindingVar->binding;
                    writeDescriptorSet.dstArrayElement = vulkanBindingVar->arrayElement;

                    switch (vulkanBindingVar->descriptorType)
                    {
                        case DescriptorType::SampledImage:
                        case DescriptorType::StorageImage:
                        {
                            descriptorSet->descriptorImageInfos[b].imageLayout = vulkanBindingVar->descriptorType == DescriptorType::SampledImage
                                                                                     ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                                                                     : VK_IMAGE_LAYOUT_GENERAL;

                            if (vulkanBindingVar->textureView)
                            {
                                descriptorSet->descriptorImageInfos[b].imageView = vulkanBindingVar->textureView->imageView;
                            }
                            else if (vulkanBindingVar->texture)
                            {
                                descriptorSet->descriptorImageInfos[b].imageView = static_cast<VulkanTextureView*>(vulkanBindingVar->texture->textureView.handler)->imageView;
                            }
                            else
                            {
                                //TODO get default texture.
                                //descriptorSet->descriptorImageInfos[b].imageView = static_cast<VulkanTextureView*>(static_cast<VulkanTexture*>(context.defaultTexture.handler)->textureView.handler)->imageView;
                            }

                            writeDescriptorSet.pImageInfo = &descriptorSet->descriptorImageInfos[b];
                            break;
                        }
                        case DescriptorType::Sampler:
                        {
                            descriptorSet->descriptorImageInfos[b].sampler = vulkanBindingVar->sampler
                                                                                 ? vulkanBindingVar->sampler->sampler
                                                                                 : static_cast<VulkanSampler*>(Graphics::GetDefaultSampler().handler)->sampler;
                            writeDescriptorSet.pImageInfo = &descriptorSet->descriptorImageInfos[b];
                            break;
                        }
                        case DescriptorType::UniformBuffer:
                        case DescriptorType::StorageBuffer:
                            break;
                        case DescriptorType::AccelerationStructure:
                            break;
                    }
                }
                vkUpdateDescriptorSets(vulkanDevice.device, descriptorSet->descriptorWrites.Size(), descriptorSet->descriptorWrites.Data(), 0, nullptr);
                data.dirty = false;
            }

            vkCmdBindDescriptorSets(cmd.commandBuffer,
                                    vulkanPipelineState->bindingPoint,
                                    vulkanPipelineState->layout,
                                    descriptorIt.first,
                                    1,
                                    &data.descriptorSet,
                                    0,
                                    nullptr);
        }
    }
}
