#pragma once
#include "Fyrion/Core/HashMap.hpp"
#include "Fyrion/Core/SharedPtr.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"

#include "volk.h"
#include "VulkanTypes.hpp"
#include "Fyrion/Core/FixedArray.hpp"

namespace Fyrion
{
    struct VulkanBindingVar;
    struct VulkanCommands;
    struct VulkanBindingSet;
    class VulkanDevice;


    struct VulkanDescriptorSetData
    {
        u8                    frame{};
        VkDescriptorSetLayout descriptorSetLayout{};
        VkDescriptorSet       descriptorSet{};
        bool                  dirty = true;
    };

    struct VulkanDescriptorSet
    {
        VulkanDevice& vulkanDevice;

        FixedArray<u8, FY_FRAMES_IN_FLIGHT> frames{0, 0};
        Array<VulkanDescriptorSetData>      data;

        Array<VulkanBindingVar*>      bindingVars{};
        Array<VkWriteDescriptorSet>   descriptorWrites;
        Array<VkDescriptorImageInfo>  descriptorImageInfos;
        Array<VkDescriptorBufferInfo> descriptorBufferInfos;

        void MarkDirty();
    };

    struct VulkanBindingVar : BindingVar
    {
        VulkanBindingSet& bindingSet;

        SharedPtr<VulkanDescriptorSet> descriptorSet{};
        u32                            binding{};
        u32                            arrayElement{};
        DescriptorType                 descriptorType{};
        u32                            size{};

        VulkanBindingVar(VulkanBindingSet& bindingSet) : bindingSet(bindingSet) {}
        ~VulkanBindingVar() override;

        VulkanTexture*      texture{};
        VulkanTextureView*  textureView{};
        VulkanSampler*      sampler{};
        VulkanBuffer*       buffer{};      //external buffers, BindingSet don't own it
        Array<VulkanBuffer> valueBuffer{}; //internal buffers created for "SetValue"

        void SetTexture(const Texture& texture) override;
        void SetTextureView(const TextureView& textureView) override;
        void SetSampler(const Sampler& sampler) override;
        void SetBuffer(const Buffer& buffer) override;
        void SetValue(ConstPtr ptr, usize size) override;
    };

    struct VulkanBindingSet : BindingSet
    {
        VulkanDevice& vulkanDevice;
        ShaderAsset*  shaderAsset;

        //shader reflection data
        HashMap<String, u32>           valueDescriptorSetLookup{};
        HashMap<u32, DescriptorLayout> descriptorLayoutLookup{};

        //binding set values
        HashMap<String, SharedPtr<VulkanBindingVar>> bindingValues;

        //runtime vulkan data
        HashMap<u32, SharedPtr<VulkanDescriptorSet>> descriptorSets{};

        VulkanBindingSet(ShaderAsset* shaderAsset, VulkanDevice& vulkanDevice);
        ~VulkanBindingSet() override;

        BindingVar* GetVar(const StringView& name) override;
        void        Reload() override;

        void Bind(VulkanCommands& cmd, const PipelineState& pipeline);
    };
}
