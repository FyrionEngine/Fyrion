#pragma once

#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion::Graphics
{
    FY_API Span<Adapter>   GetAdapters();
    FY_API Swapchain       CreateSwapchain(const SwapchainCreation& swapchainCreation);
    FY_API RenderPass      CreateRenderPass(const RenderPassCreation& renderPassCreation);
    FY_API Buffer          CreateBuffer(const BufferCreation& bufferCreation);
    FY_API Texture         CreateTexture(const TextureCreation& textureCreation);
    FY_API TextureView     CreateTextureView(const TextureViewCreation& textureViewCreation);
    FY_API Sampler         CreateSampler(const SamplerCreation& samplerCreation);
    FY_API PipelineState   CreateGraphicsPipelineState(const GraphicsPipelineCreation& graphicsPipelineCreation);
    FY_API PipelineState   CreateComputePipelineState(const ComputePipelineCreation& computePipelineCreation);
    FY_API BindingSet*     CreateBindingSet(ShaderAsset* shaderAsset);
    FY_API BindingSet*     CreateBindingSet(Span<DescriptorLayout> descriptorLayouts);
    FY_API void            DestroySwapchain(const Swapchain& swapchain);
    FY_API void            DestroyRenderPass(const RenderPass& renderPass);
    FY_API void            DestroyBuffer(const Buffer& buffer);
    FY_API void            DestroyTexture(const Texture& texture);
    FY_API void            DestroyTextureView(const TextureView& textureView);
    FY_API void            DestroySampler(const Sampler& sampler);
    FY_API void            DestroyGraphicsPipelineState(const PipelineState& pipelineState);
    FY_API void            DestroyComputePipelineState(const PipelineState& pipelineState);
    FY_API void            DestroyBindingSet(BindingSet* bindingSet);
    FY_API RenderPass      AcquireNextRenderPass(Swapchain swapchain);
    FY_API void            WaitQueue();
    FY_API void            UpdateBufferData(const BufferDataInfo& bufferDataInfo);
    FY_API void            UpdateTextureData(const TextureDataInfo& textureDataInfo);
    FY_API void            UpdateTextureLayout(Texture texture, ResourceLayout oldLayout, ResourceLayout newLayout, bool isDepth = false);
    FY_API VoidPtr         GetBufferMappedMemory(const Buffer& buffer);
    FY_API void            GetTextureData(const TextureGetDataInfo& info, Array<u8>& data);
    FY_API TextureCreation GetTextureCreationInfo(Texture texture);
    FY_API RenderCommands& GetCmd();
    FY_API GPUQueue        GetMainQueue();
    FY_API RenderApiType   GetRenderApi();
    FY_API Sampler         GetDefaultSampler();
    FY_API Texture         GetDefaultTexture();
    FY_API void            AddTask(GraphicsTaskType graphicsTask, VoidPtr userData, FnGraphicsTask task);
}
