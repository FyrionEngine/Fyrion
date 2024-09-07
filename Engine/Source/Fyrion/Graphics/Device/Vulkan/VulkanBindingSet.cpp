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
        shaderAsset->AddBindingSetDependency(this);
        LoadInfo();
    }

    void VulkanBindingSet::Reload()
    {
        for (auto& descriptorIt : descriptorSets)
        {
            for (auto& data : descriptorIt.second->data)
            {
                vkDestroyDescriptorSetLayout(vulkanDevice.device, data.descriptorSetLayout, nullptr);
            }
        }

        descriptorSets.Clear();
        valueDescriptorSetLookup.Clear();
        descriptorLayoutLookup.Clear();

        LoadInfo();

        //it should recreate the bindingVars but keep the original values
        //note: never remove a bindingVar, even if it's not present anymore in the shader.
        auto oldBindingVar = Traits::Move(bindingVars);
        for(const auto& bindingVarIt: oldBindingVar)
        {
            VulkanBindingVar* oldVar = bindingVarIt.second.Get();

            VulkanBindingVar* newVar = static_cast<VulkanBindingVar*>(GetVar(bindingVarIt.first));
            newVar->texture = oldVar->texture;
            newVar->textureView = oldVar->textureView;
            newVar->sampler = oldVar->sampler;
            newVar->buffer = oldVar->buffer;
            newVar->valueBuffer = Traits::Move(oldVar->valueBuffer);
        }
    }

    void VulkanBindingSet::LoadInfo()
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
        shaderAsset->RemoveBindingSetDependency(this);

        for (auto& descriptorIt : descriptorSets)
        {
            for (auto& data : descriptorIt.second->data)
            {
                vkDestroyDescriptorSetLayout(vulkanDevice.device, data.descriptorSetLayout, nullptr);
                vkFreeDescriptorSets(vulkanDevice.device, vulkanDevice.descriptorPool, 1, &data.descriptorSet);
            }
        }
    }

    void VulkanDescriptorSet::MarkDirty()
    {
        data[frames[vulkanDevice.currentFrame]].dirty = true;
    }

    VulkanBindingVar::~VulkanBindingVar()
    {
        for(VulkanBuffer& vulkanBuffer : valueBuffer)
        {
            if (vulkanBuffer.buffer && vulkanBuffer.allocation)
            {
                vmaDestroyBuffer(descriptorSet->vulkanDevice.vmaAllocator, vulkanBuffer.buffer, vulkanBuffer.allocation);
            }
        }
    }

    void VulkanBindingVar::SetTexture(const Texture& p_texture)
    {
        VulkanTexture* newTexture = static_cast<VulkanTexture*>(p_texture.handler);
        if (newTexture != nullptr)
        {
            if (texture == nullptr || texture->id != newTexture->id)
            {
                texture = newTexture;
                MarkDirty();
            }
        }
        else if (texture != nullptr)
        {
            texture = {};
            MarkDirty();
        }

    }

    void VulkanBindingVar::SetTextureView(const TextureView& p_textureView)
    {
        if (textureView != p_textureView.handler)
        {
            textureView = static_cast<VulkanTextureView*>(p_textureView.handler);
            MarkDirty();
        }
    }

    void VulkanBindingVar::SetSampler(const Sampler& p_sampler)
    {
        if (sampler != p_sampler.handler)
        {
            sampler = static_cast<VulkanSampler*>(p_sampler.handler);
            MarkDirty();
        }
    }

    void VulkanBindingVar::SetBuffer(const Buffer& p_buffer)
    {
        if (buffer != p_buffer.handler)
        {
            buffer = static_cast<VulkanBuffer*>(p_buffer.handler);
            MarkDirty();
        }
    }

    void VulkanBindingVar::SetValue(ConstPtr ptr, usize size)
    {
        //TODO check if that's a different frame? and if there is already
        if (valueBuffer.Empty())
        {
            VkBufferCreateInfo bufferInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            bufferInfo.size = size;
            bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferInfo.usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            VmaAllocationCreateInfo vmaAllocInfo = {};
            vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            vmaAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

            VulkanBuffer& vulkanBuffer = valueBuffer.EmplaceBack();
            vulkanBuffer.bufferCreation.size = size;

            vmaCreateBuffer(descriptorSet->vulkanDevice.vmaAllocator,
                &bufferInfo,
                &vmaAllocInfo,
                &vulkanBuffer.buffer,
                &vulkanBuffer.allocation,
                &vulkanBuffer.allocInfo);
        }

        VulkanBuffer& vulkanBuffer =  valueBuffer[0];
        char* memory = static_cast<char*>(vulkanBuffer.allocInfo.pMappedData);
        MemCopy(memory, ptr, size);
    }

    void VulkanBindingVar::MarkDirty()
    {
        if (descriptorSet)
        {
            descriptorSet->MarkDirty();
        }
    }

    BindingVar* VulkanBindingSet::GetVar(const StringView& name)
    {
        auto it = bindingVars.Find(name);
        if (it == bindingVars.end())
        {
            //find the attribute descriptor set number
            auto varDescriptorSetIt = valueDescriptorSetLookup.Find(name);
            if (varDescriptorSetIt == valueDescriptorSetLookup.end())
            {
                return bindingVars.Emplace(name, MakeShared<VulkanBindingVar>(*this)).first->second.Get();
            }

            u32 set = varDescriptorSetIt->second;

            auto descriptorSetIt = descriptorSets.Find(set);

            //if there is no VulkanDescriptorSet for the attribute, create one.
            if (descriptorSetIt == descriptorSets.end())
            {
                descriptorSetIt = descriptorSets.Emplace(set, MakeShared<VulkanDescriptorSet>(vulkanDevice)).first;
            }

            DescriptorLayout& descriptorLayout = descriptorLayoutLookup[set];

            SharedPtr<VulkanDescriptorSet> vulkanDescriptorSet = descriptorSetIt->second;


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
                    //TODO -- needs a pool of "descriptor pool"
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

                    VulkanBindingVar* bindingVar = bindingVars.Emplace(descriptorBinding.name, MakeShared<VulkanBindingVar>(*this)).first->second.Get();
                    bindingVar->descriptorSet = vulkanDescriptorSet;
                    bindingVar->binding = descriptorBinding.binding;
                    bindingVar->descriptorType = descriptorBinding.descriptorType;
                    bindingVar->size = descriptorBinding.size;
                    vulkanDescriptorSet->bindingVars[i] = bindingVar;
                }
            }

            it = bindingVars.Find(name);
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
                            bool depthFormat = false;

                            if (vulkanBindingVar->textureView)
                            {
                                descriptorSet->descriptorImageInfos[b].imageView = vulkanBindingVar->textureView->imageView;
                            }
                            else if (vulkanBindingVar->texture)
                            {
                                depthFormat = vulkanBindingVar->texture->creation.format == Format::Depth;
                                descriptorSet->descriptorImageInfos[b].imageView = static_cast<VulkanTextureView*>(vulkanBindingVar->texture->textureView.handler)->imageView;
                            }
                            else
                            {
                                descriptorSet->descriptorImageInfos[b].imageView = static_cast<VulkanTextureView*>(static_cast<VulkanTexture*>(Graphics::GetDefaultTexture().handler)->textureView.handler)->imageView;
                            }

                            descriptorSet->descriptorImageInfos[b].imageLayout = depthFormat ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL :
                                vulkanBindingVar->descriptorType == DescriptorType::SampledImage
                                    ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                                    : VK_IMAGE_LAYOUT_GENERAL;

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

                            if (vulkanBindingVar->buffer)
                            {
                                descriptorSet->descriptorBufferInfos[b].offset = 0;
                                descriptorSet->descriptorBufferInfos[b].buffer = vulkanBindingVar->buffer->buffer;
                                descriptorSet->descriptorBufferInfos[b].range = vulkanBindingVar->buffer->bufferCreation.size;
                            }
                            else if (!vulkanBindingVar->valueBuffer.Empty())
                            {
                                VulkanBuffer& vulkanBuffer = vulkanBindingVar->valueBuffer[0];
                                descriptorSet->descriptorBufferInfos[b].offset = 0;
                                descriptorSet->descriptorBufferInfos[b].buffer = vulkanBuffer.buffer;
                                descriptorSet->descriptorBufferInfos[b].range = vulkanBuffer.bufferCreation.size;
                            }
                            else
                            {
                                //TODO make a default buffer?
                            }
                            writeDescriptorSet.pBufferInfo = &descriptorSet->descriptorBufferInfos[b];
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
