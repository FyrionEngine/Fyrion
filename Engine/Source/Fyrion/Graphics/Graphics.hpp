#pragma once

#include "Fyrion/Graphics/GraphicsTypes.hpp"

namespace Fyrion::Graphics
{
    FY_API Span<Adapter> GetAdapters();
    FY_API Swapchain     CreateSwapchain(const SwapchainCreation& swapchainCreation);
    FY_API RenderPass    CreateRenderPass(const RenderPassCreation& renderPassCreation);
    FY_API Buffer        CreateBuffer(const BufferCreation& bufferCreation);
    FY_API Texture       CreateTexture(const TextureCreation& textureCreation);
    FY_API TextureView   CreateTextureView(const TextureViewCreation& textureViewCreation);
    FY_API Sampler       CreateSampler(const SamplerCreation& samplerCreation);
    FY_API PipelineState CreateGraphicsPipelineState(const GraphicsPipelineCreation& graphicsPipelineCreation);
    FY_API PipelineState CreateComputePipelineState(const ComputePipelineCreation& computePipelineCreation);
    FY_API BindingSet*   CreateBindingSet(ShaderAsset* shaderAsset);
    FY_API void          DestroySwapchain(const Swapchain& swapchain);
    FY_API void          DestroyRenderPass(const RenderPass& renderPass);
    FY_API void          DestroyBuffer(const Buffer& buffer);
    FY_API void          DestroyTexture(const Texture& texture);
    FY_API void          DestroyTextureView(const TextureView& textureView);
    FY_API void          DestroySampler(const Sampler& sampler);
    FY_API void          DestroyGraphicsPipelineState(const PipelineState& pipelineState);
    FY_API void          DestroyComputePipelineState(const PipelineState& pipelineState);
    FY_API void          DestroyBindingSet(BindingSet* bindingSet);
    FY_API RenderPass    AcquireNextRenderPass(Swapchain swapchain);
    FY_API void          WaitQueue();
    FY_API void          UpdateBufferData(const BufferDataInfo& bufferDataInfo);
    FY_API RenderApiType GetRenderApi();
}
