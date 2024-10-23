#include "RenderGraph.hpp"

#include "Graphics.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Graph.hpp"
#include "Fyrion/Core/Logger.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::RenderGraph");
    }

    void RenderGraphResource::WriteIn(RenderGraphPass* pass)
    {
        if (!edges.Empty() && edges.Back().writePass == nullptr)
        {
            edges.Back().writePass = pass;
            return;
        }

        edges.EmplaceBack(ResourceEdges{.writePass = pass});
    }

    void RenderGraphResource::ReadIn(RenderGraphPass* pass)
    {
        if (edges.Empty())
        {
            edges.EmplaceBack(ResourceEdges{});
        }
        edges.Back().readPass.EmplaceBack(pass);
    }

    RenderGraphResource::~RenderGraphResource()
    {
        if (texture && (creation.type == RenderGraphResourceType::Texture || creation.type == RenderGraphResourceType::Attachment))
        {
            Graphics::DestroyTexture(texture);
        }

        if (buffer && creation.type == RenderGraphResourceType::Buffer)
        {
            Graphics::DestroyBuffer(buffer);
        }
    }

    RenderPass RenderGraphPass::GetRenderPass() const
    {
        return renderPass;
    }

    StringView RenderGraphPass::GetName() const
    {
        return name;
    }

    RenderGraphPass::~RenderGraphPass()
    {
        if (handler)
        {
            handler->Destroy();
        }

        if (renderPass)
        {
            Graphics::DestroyRenderPass(renderPass);
        }
    }

    void RenderGraphPass::CreateRenderPass()
    {
        if (type == RenderGraphPassType::Graphics)
        {
            LoadOp loadOp = clearDepth || clearValue ? LoadOp::Clear : LoadOp::Load;

            Array<AttachmentCreation> attachments{};
            for (const auto& output : outputs)
            {
                if (output.resource->creation.type == RenderGraphResourceType::Attachment)
                {
                    AttachmentCreation attachmentCreation = AttachmentCreation{
                        .texture = output.resource->texture,
                        .loadOp = loadOp
                    };

                    switch (loadOp)
                    {
                        case LoadOp::Load:
                        {
                            attachmentCreation.initialLayout = output.resource->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                            attachmentCreation.finalLayout = output.resource->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                            break;
                        }
                        case LoadOp::DontCare:
                        case LoadOp::Clear:
                        {
                            attachmentCreation.initialLayout = ResourceLayout::Undefined;
                            attachmentCreation.finalLayout = output.resource->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                            break;
                        }
                    }

                    attachments.EmplaceBack(attachmentCreation);

                    extent = output.resource->textureCreation.extent;
                }
            }

            RenderPassCreation renderPassCreation{
                .attachments = attachments
            };

            renderPass = Graphics::CreateRenderPass(renderPassCreation);
        }
    }

    RenderPassBuilder::RenderPassBuilder(RenderGraph* rg, RenderGraphPass* pass) : rg(rg), pass(pass) {}

    RenderPassBuilder& RenderPassBuilder::Read(RenderGraphResource* resource)
    {
        resource->ReadIn(pass);
        pass->inputs.EmplaceBack(resource);
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::Write(RenderGraphResource* resource)
    {
        resource->WriteIn(pass);
        pass->outputs.EmplaceBack(resource);
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::ClearColor(const Vec4& color)
    {
        pass->clearValue = color;
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::ClearDepth(bool clear)
    {
        pass->clearDepth = clear;
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::Handler(RenderGraphPassHandler* handler)
    {
        handler->pass = pass;
        handler->rg = rg;
        pass->handler = handler;
        return *this;
    }


    RenderPassBuilder RenderGraph::AddPass(StringView name, RenderGraphPassType type)
    {
        SharedPtr<RenderGraphPass> pass = MakeShared<RenderGraphPass>();
        pass->id = passes.Size() + 1;
        pass->name = name;
        pass->type = type;
        passes.EmplaceBack(pass);
        return {this, pass.Get()};
    }

    RenderGraphResource* RenderGraph::Create(const RenderGraphResourceCreation& creation)
    {
        SharedPtr<RenderGraphResource> resource = MakeShared<RenderGraphResource>(creation);
        resources.EmplaceBack(resource);
        return resource.Get();
    }

    void RenderGraph::Resize(Extent extent)
    {
        this->viewportExtent = extent;
        Graphics::WaitQueue();

        for (auto& resource : resources)
        {
            if ((resource->creation.type == RenderGraphResourceType::Texture || resource->creation.type == RenderGraphResourceType::Attachment)
                && resource->creation.scale > 0.f)
            {
                Texture oldTexture = resource->texture;

                Extent size = Extent{viewportExtent.width, viewportExtent.height} * resource->creation.scale;
                resource->textureCreation.extent = {(size.width), (size.height), 1};
                resource->textureCreation.name = resource->creation.name;
                resource->texture = Graphics::CreateTexture(resource->textureCreation);

                //defer destroy to avoid getting the same pointer address for the next texture
                if (oldTexture)
                {
                    Graphics::DestroyTexture(oldTexture);
                }

                if (resource->creation.type == RenderGraphResourceType::Texture)
                {
                    Graphics::UpdateTextureLayout(resource->texture, ResourceLayout::Undefined, ResourceLayout::ShaderReadOnly);
                    resource->currentLayout = ResourceLayout::ShaderReadOnly;
                }
            }
        }

        for (auto& pass : passes)
        {
            if (pass->type == RenderGraphPassType::Graphics)
            {
                if (pass->renderPass)
                {
                    Graphics::DestroyRenderPass(pass->renderPass);
                }
                pass->CreateRenderPass();
            }

            if (pass->handler)
            {
                pass->handler->Resize(pass->extent);
            }
        }
    }

    void RenderGraph::Bake(Extent extent)
    {
        this->viewportExtent = extent;

        FY_ASSERT(this->colorOutput, "color output must be provided");
        FY_ASSERT(this->depthOutput, "depth output must be provided");

        CreateResources();

        Graph<u32, SharedPtr<RenderGraphPass>> graph{};
        for (auto& pass : passes)
        {
            graph.AddNode(pass->id, pass);
        }

        for (auto& resource : resources)
        {
            for (auto& edge : resource->edges)
            {
                for (auto& read : edge.readPass)
                {
                    graph.AddEdge(read->id, edge.writePass->id);
                }
            }
        }

        passes = graph.Sort();

        for (auto& pass : passes)
        {
            pass->CreateRenderPass();

            if (pass->handler)
            {
                pass->handler->Init();
            }

            logger.Debug("pass {} created ", pass->name);
        }

        Event::Bind<OnRecordRenderCommands, &RenderGraph::RecordCommands>(this);
    }

    Extent RenderGraph::GetViewportExtent() const
    {
        return viewportExtent;
    }

    void RenderGraph::SetCameraData(const CameraData& cameraData)
    {
        this->cameraData = cameraData;
    }

    const CameraData& RenderGraph::GetCameraData() const
    {
        return this->cameraData;
    }

    void RenderGraph::ColorOutput(RenderGraphResource* resource)
    {
        this->colorOutput = resource;
    }

    void RenderGraph::DepthOutput(RenderGraphResource* resource)
    {
        this->depthOutput = resource;
    }

    Texture RenderGraph::GetColorOutput() const
    {
        if(colorOutput)
        {
            return colorOutput->texture;
        }
        return {};
    }

    Texture RenderGraph::GetDepthOutput() const
    {
        if(depthOutput)
        {
            return depthOutput->texture;
        }
        return {};
    }

    void RenderGraph::RecordCommands(RenderCommands& cmd, f64 deltaTime)
    {
        for (auto& pass : passes)
        {
            if (pass->handler)
            {
                pass->handler->Update(deltaTime);
            }

            cmd.BeginLabel(pass->name, {0, 0, 0, 1});
            for (const auto& input : pass->inputs)
            {
                if (input.resource->creation.type == RenderGraphResourceType::Texture)
                {
                    ResourceLayout newLayout = input.resource->creation.format != Format::Depth ? ResourceLayout::ShaderReadOnly : ResourceLayout::DepthStencilReadOnly;
                    if (input.resource->currentLayout != newLayout)
                    {
                        ResourceBarrierInfo resourceBarrierInfo{};
                        resourceBarrierInfo.texture = input.resource->texture;
                        resourceBarrierInfo.oldLayout = input.resource->currentLayout;
                        resourceBarrierInfo.newLayout = newLayout;
                        cmd.ResourceBarrier(resourceBarrierInfo);

                        input.resource->currentLayout = newLayout;
                    }
                }
            }

            for (const auto& output : pass->outputs)
            {
                if (output.resource->creation.type == RenderGraphResourceType::Texture)
                {
                    if (output.resource->currentLayout != ResourceLayout::General)
                    {
                        ResourceBarrierInfo resourceBarrierInfo{};
                        resourceBarrierInfo.texture = output.resource->texture;
                        resourceBarrierInfo.oldLayout = output.resource->currentLayout;
                        resourceBarrierInfo.newLayout = ResourceLayout::General;
                        cmd.ResourceBarrier(resourceBarrierInfo);

                        output.resource->currentLayout = ResourceLayout::General;
                    }
                }
            }

            if (pass->renderPass)
            {
                BeginRenderPassInfo renderPassInfo{};
                renderPassInfo.renderPass = pass->renderPass;

                if (pass->clearValue)
                {
                    renderPassInfo.clearValue = &*pass->clearValue;
                }

                ClearDepthStencilValue clearDepthStencilValue{};

                if (pass->clearDepth)
                {
                    renderPassInfo.depthStencil = &clearDepthStencilValue;
                }

                cmd.BeginRenderPass(renderPassInfo);

                ViewportInfo viewportInfo{};
                viewportInfo.x = 0.;
                viewportInfo.y = 0.;
                viewportInfo.y = (f32)pass->extent.height;
                viewportInfo.width = (f32)pass->extent.width;
                viewportInfo.height = -(f32)pass->extent.height;
                viewportInfo.minDepth = 0.;
                viewportInfo.maxDepth = 1.;

                cmd.SetViewport(viewportInfo);

                auto scissor = Rect{0, 0, pass->extent.width, pass->extent.height};
                cmd.SetScissor(scissor);
            }

            if (pass->handler)
            {
                pass->handler->Render(cmd);
            }

            if (pass->renderPass)
            {
                cmd.EndRenderPass();

                for (auto output : pass->outputs)
                {
                    if (output.resource->creation.type == RenderGraphResourceType::Attachment)
                    {
                        output.resource->currentLayout = output.resource->textureCreation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                    }
                }
            }

            cmd.EndLabel();
        }

        if (colorOutput && colorOutput->currentLayout != ResourceLayout::ShaderReadOnly)
        {
            ResourceBarrierInfo resourceBarrierInfo{};
            resourceBarrierInfo.texture = colorOutput->texture;
            resourceBarrierInfo.oldLayout = colorOutput->currentLayout;
            resourceBarrierInfo.newLayout = ResourceLayout::ShaderReadOnly;
            cmd.ResourceBarrier(resourceBarrierInfo);

            colorOutput->currentLayout = ResourceLayout::ShaderReadOnly;
        }

        if (depthOutput && depthOutput->currentLayout != ResourceLayout::DepthStencilReadOnly)
        {
            ResourceBarrierInfo resourceBarrierInfo{};
            resourceBarrierInfo.texture = depthOutput->texture;
            resourceBarrierInfo.oldLayout = depthOutput->currentLayout;
            resourceBarrierInfo.newLayout = ResourceLayout::DepthStencilReadOnly;
            cmd.ResourceBarrier(resourceBarrierInfo);

            depthOutput->currentLayout = ResourceLayout::DepthStencilReadOnly;
        }
    }

    void RenderGraph::CreateResources()
    {
        for (auto& resource : resources)
        {
            switch (resource->creation.type)
            {
                case RenderGraphResourceType::None:
                    break;
                case RenderGraphResourceType::Buffer:
                {
                    if (resource->creation.bufferInitialSize > 0)
                    {
                        resource->buffer = Graphics::CreateBuffer(BufferCreation{
                            .usage = resource->creation.bufferUsage,
                            .size = resource->creation.bufferInitialSize,
                            .allocation = resource->creation.bufferAllocation
                        });
                    }
                    break;
                }
                case RenderGraphResourceType::Texture:
                case RenderGraphResourceType::Attachment:
                {
                    if (resource->creation.size > 0)
                    {
                        resource->textureCreation.extent = resource->creation.size;
                    }
                    else if (resource->creation.scale > 0.f)
                    {
                        Extent size = Extent{viewportExtent.width, viewportExtent.height} * resource->creation.scale;
                        resource->textureCreation.extent = {size.width, size.height, 1};
                    }
                    else
                    {
                        FY_ASSERT(false, "texture without size");
                    }

                    resource->textureCreation.format = resource->creation.format;

                    if (resource->textureCreation.format != Format::Depth)
                    {
                        if (resource->creation.type == RenderGraphResourceType::Attachment)
                        {
                            resource->textureCreation.usage = TextureUsage::RenderPass | TextureUsage::ShaderResource;
                        }
                        else if (resource->creation.type == RenderGraphResourceType::Texture)
                        {
                            resource->textureCreation.usage = TextureUsage::Storage | TextureUsage::ShaderResource;
                        }
                    }
                    else
                    {
                        resource->textureCreation.usage = TextureUsage::DepthStencil | TextureUsage::ShaderResource;
                    }

                    resource->texture = Graphics::CreateTexture(resource->textureCreation);

                    if (resource->creation.type == RenderGraphResourceType::Texture)
                    {
                        Graphics::UpdateTextureLayout(resource->texture, ResourceLayout::Undefined, ResourceLayout::ShaderReadOnly);
                        resource->currentLayout = ResourceLayout::ShaderReadOnly;
                    }
                    break;
                }
                case RenderGraphResourceType::Reference:
                {
                    break;
                }
            }
            logger.Debug("Created resource {} ", resource->creation.name);
        }
    }
}
