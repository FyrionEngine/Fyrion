#include "RenderGraph.hpp"

#include "Graphics.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Graph.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::RenderGraph");
        bool registerSwapchainRenderEvent = true;
        HashMap<String, RenderGraphPassCreation> passDatabase{};
    }

    void RenderGraph::SetRegisterSwapchainRenderEvent(bool p_registerSwapchainRenderEvent)
    {
        registerSwapchainRenderEvent = p_registerSwapchainRenderEvent;
    }

    void RenderGraph::RegisterPass(const RenderGraphPassCreation& renderGraphPassCreation)
    {
        logger.Debug("pass {} Registered", renderGraphPassCreation.name);
        passDatabase.Emplace(renderGraphPassCreation.name, RenderGraphPassCreation{renderGraphPassCreation});
    }

    void RenderGraphAsset::RegisterType(NativeTypeHandler<RenderGraphAsset>& type)
    {
        type.Field<&RenderGraphAsset::passes>("passes");
        type.Field<&RenderGraphAsset::edges>("edges");
        type.Field<&RenderGraphAsset::colorOutput>("colorOutput");
        type.Field<&RenderGraphAsset::depthOutput>("depthOutput");
    }

    Array<String> RenderGraphAsset::GetPasses() const
    {
        return passes;
    }

    Array<RenderGraphEdge> RenderGraphAsset::GetEdges() const
    {
        return edges;
    }

    StringView RenderGraphAsset::GetColorOutput() const
    {
        return colorOutput;
    }

    StringView RenderGraphAsset::GetDepthOutput() const
    {
        return depthOutput;
    }

    RenderGraphResource::~RenderGraphResource()
    {
        if (texture && (creation.type == RenderGraphResourceType::Texture || creation.type == RenderGraphResourceType::Attachment))
        {
            Graphics::DestroyTexture(texture);
        }

        if (buffer)
        {
            Graphics::DestroyBuffer(buffer);
        }
    }

    RenderGraphNode::~RenderGraphNode()
    {
        if (renderGraphPass && renderGraphPassTypeHandler)
        {
            renderGraphPass->Destroy();
            renderGraphPassTypeHandler->Destroy(renderGraphPass);
        }

        if (renderPass)
        {
            Graphics::DestroyRenderPass(renderPass);
        }
    }

    RenderPass RenderGraphNode::GetRenderPass() const
    {
        return renderPass;
    }

    Texture RenderGraphNode::GetInputTexture(StringView view) const
    {
        if (auto it = inputs.Find(view))
        {
            return it->second->resource->texture;
        }

        return {};
    }

    Texture RenderGraphNode::GetOutputTexture(StringView view) const
    {
        if (auto it = outputs.Find(view))
        {
            return it->second->texture;
        }

        return {};
    }

    RenderGraphResource* RenderGraphNode::GetInputResource(StringView view) const
    {
        if (auto it = inputs.Find(view))
        {
            return it->second->resource.Get();
        }

        return {};
    }

    RenderGraphResource* RenderGraphNode::GetOutputResource(StringView view) const
    {
        if (auto it = outputs.Find(view))
        {
            return it->second.Get();
        }

        return {};
    }

    void RenderGraphNode::CreateRenderPass()
    {
        if (creation.type == RenderGraphPassType::Graphics)
        {
            Array<AttachmentCreation> attachments{};
            for (const auto& output : creation.outputs)
            {
                if (output.type == RenderGraphResourceType::Attachment)
                {
                    if (auto it = outputs.Find(output.name))
                    {
                        AttachmentCreation attachmentCreation = AttachmentCreation{
                            .texture = it->second->texture,
                            .loadOp = output.loadOp
                        };

                        switch (output.loadOp)
                        {
                            case LoadOp::Load:
                            {
                                attachmentCreation.initialLayout = it->second->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                                attachmentCreation.finalLayout = it->second->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                                break;
                            }
                            case LoadOp::DontCare:
                            case LoadOp::Clear:
                            {
                                attachmentCreation.initialLayout = ResourceLayout::Undefined;
                                attachmentCreation.finalLayout = it->second->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                                break;
                            }
                        }

                        attachments.EmplaceBack(attachmentCreation);
                        extent = it->second->textureCreation.extent;

                        if (output.loadOp == LoadOp::Clear)
                        {
                            if (!clearColor)
                            {
                                clearColor = MakeOptional<Vec4>(output.clearValue);
                            }
                        }

                        if (output.clearDepth)
                        {
                            if (!clearDepthStencil)
                            {
                                clearDepthStencil = MakeOptional<ClearDepthStencilValue>(ClearDepthStencilValue{1.0, 0});
                            }
                        }
                    }
                }
            }

            RenderPassCreation renderPassCreation{
                .attachments = attachments
            };

            renderPass = Graphics::CreateRenderPass(renderPassCreation);
        }
    }

    RenderGraph::RenderGraph(RenderGraphAsset* asset) : asset(asset)
    {
        viewportExtent = Engine::GetViewportExtent();
        Create();
    }

    RenderGraph::RenderGraph(Extent extent, RenderGraphAsset* asset): asset(asset), viewportExtent(extent)
    {
        Create();
    }

    RenderGraph::~RenderGraph()
    {
        Graphics::WaitQueue();

        Event::Unbind<OnRecordRenderCommands, &RenderGraph::RecordCommands>(this);

        if (registerSwapchainRenderEvent)
        {
            Event::Unbind<OnSwapchainRender, &RenderGraph::BlitSwapchapin>(this);
        }
    }

    void RenderGraph::Create()
    {
        if (!asset) return;

        Graph<String, SharedPtr<RenderGraphNode>> graph{};

        for (const String& pass : asset->GetPasses())
        {
            if (const auto it = passDatabase.Find(pass))
            {
                graph.AddNode(pass, MakeShared<RenderGraphNode>(it->first, it->second));
                continue;
            }
            logger.Error("pass {} not found", pass);
        }

        for (const auto& edge: asset->GetEdges())
        {
            graph.AddEdge(edge.nodeInput, edge.nodeOutput);
        }

        nodes = graph.Sort();


        for(auto& node: nodes)
        {
            logger.Debug("node {} created ", node->name);
        }

        for (SharedPtr<RenderGraphNode>& node : nodes)
        {
            for (auto& input : node->creation.inputs)
            {
                for (const auto& edge: asset->GetEdges())
                {
                    if (edge.nodeInput == node->name && edge.input == input.name)
                    {
                        String outputName = edge.nodeOutput + "#" + edge.output;
                        if (auto itResource = resources.Find(outputName))
                        {
                            String inputName = node->name + "#" + input.name;
                            node->inputs.Insert(input.name, MakeShared<RenderGraphInput>(
                                inputName,
                                input,
                                itResource->second));
                        }
                    }
                }
            }

            for(const auto& output: node->creation.outputs)
            {
                String resourceName = node->name + "#" + output.name;

                //TODO what if the output has the same name but different configs?
                if (auto it = node->inputs.Find(output.name))
                {
                    node->outputs.Insert(output.name, it->second->resource);
                    resources.Insert(resourceName, it->second->resource);
                    continue;
                }

                SharedPtr<RenderGraphResource> resource = CreateResource(resourceName, output);
                node->outputs.Insert(output.name, resource);
            }

            node->CreateRenderPass();

            TypeHandler* typeHandler = Registry::FindTypeByName(node->name);
            if (typeHandler)
            {
                for (TypeID baseType : typeHandler->GetBaseTypes())
                {
                    if (baseType == GetTypeID<RenderGraphPass>())
                    {
                        RenderGraphPass* renderGraphPass = typeHandler->Cast<RenderGraphPass>(typeHandler->NewInstance());
                        renderGraphPass->node = node.Get();
                        renderGraphPass->graph = this;
                        node->renderGraphPass = renderGraphPass;
                        node->renderGraphPassTypeHandler = typeHandler;
                    }
                }
            }

            if (node->renderGraphPass)
            {
                node->renderGraphPass->Init();
            }

            logger.Debug("node {} created ", node->name);
        }

        if (auto it = resources.Find(asset->GetColorOutput()))
        {
            colorOutput = it->second;
        }

        if (auto it = resources.Find(asset->GetDepthOutput()))
        {
            depthOutput = it->second;
        }

        Event::Bind<OnRecordRenderCommands, &RenderGraph::RecordCommands>(this);

        if (registerSwapchainRenderEvent)
        {
            Event::Bind<OnSwapchainRender, &RenderGraph::BlitSwapchapin>(this);
        }
    }

    Extent RenderGraph::GetViewportExtent() const
    {
        return viewportExtent;
    }

    void RenderGraph::Resize(Extent p_extent)
    {
        Graphics::WaitQueue();

        viewportExtent = p_extent;

        for (auto& it : resources)
        {
            auto& resource = it.second;

            if ((resource->creation.type == RenderGraphResourceType::Texture || resource->creation.type == RenderGraphResourceType::Attachment)
                && resource->creation.scale > 0.f)
            {
                Texture oldTexture = resource->texture;

                Extent size = Extent{viewportExtent.width, viewportExtent.height} * resource->creation.scale;
                resource->textureCreation.extent = {(size.width), (size.height), 1};
                resource->textureCreation.name = resource->fullName;
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

        for (auto& node : nodes)
        {
            if (node->creation.type == RenderGraphPassType::Graphics)
            {
                if (node->renderPass)
                {
                    Graphics::DestroyRenderPass(node->renderPass);
                }
                node->CreateRenderPass();
            }

            if (node->renderGraphPass)
            {
                node->renderGraphPass->Resize(node->extent);
            }
        }
    }

    void RenderGraph::SetCameraData(const CameraData& p_cameraData)
    {
        cameraData = p_cameraData;
    }

    const CameraData& RenderGraph::GetCameraData() const
    {
        return cameraData;
    }

    Texture RenderGraph::GetColorOutput() const
    {
        if (colorOutput)
        {
            return colorOutput->texture;
        }
        return {};
    }

    Texture RenderGraph::GetDepthOutput() const
    {
        if (depthOutput)
        {
            return depthOutput->texture;
        }
        return {};
    }

    SharedPtr<RenderGraphResource> RenderGraph::CreateResource(const StringView& fullName, const RenderGraphResourceCreation& creation)
    {
        SharedPtr<RenderGraphResource> resource = MakeShared<RenderGraphResource>(fullName, creation);

        switch (creation.type)
        {
            case RenderGraphResourceType::Buffer:
            {
                if (creation.bufferInitialSize > 0)
                {
                    resource->buffer = Graphics::CreateBuffer(BufferCreation{
                        .usage = creation.bufferUsage,
                        .size = creation.bufferInitialSize,
                        .allocation = creation.bufferAllocation
                    });
                }
                break;
            }
            case RenderGraphResourceType::Texture:
            case RenderGraphResourceType::Attachment:
            {
                if (creation.size > 0)
                {
                    resource->textureCreation.extent = creation.size;
                }
                else if (creation.scale > 0.f)
                {
                    Extent size = Extent{viewportExtent.width, viewportExtent.height} * creation.scale;
                    resource->textureCreation.extent = {size.width, size.height, 1};
                }
                else
                {
                    FY_ASSERT(false, "texture without size");
                }

                resource->textureCreation.format = creation.format;

                if (resource->textureCreation.format != Format::Depth)
                {
                    if (creation.type == RenderGraphResourceType::Attachment)
                    {
                        resource->textureCreation.usage = TextureUsage::RenderPass | TextureUsage::ShaderResource;
                    }
                    else if (creation.type == RenderGraphResourceType::Texture)
                    {
                        resource->textureCreation.usage = TextureUsage::Storage | TextureUsage::ShaderResource;
                    }
                }
                else
                {
                    resource->textureCreation.usage = TextureUsage::DepthStencil | TextureUsage::ShaderResource;
                }

                resource->textureCreation.name = fullName;
                resource->texture = Graphics::CreateTexture(resource->textureCreation);

                if (creation.type == RenderGraphResourceType::Texture)
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

        resources.Insert(fullName, resource);
        logger.Debug("Created resource {} ", fullName);

        return resource;
    }

    void RenderGraph::RecordCommands(RenderCommands& cmd, f64 deltaTime)
    {
        for(auto& node: nodes)
        {
            node->renderGraphPass->Update(deltaTime);

            cmd.BeginLabel(node->name, {0, 0, 0, 1});
            for(const auto& inputIt: node->inputs)
            {
                if (inputIt.second->inputCreation.type == RenderGraphResourceType::Texture)
                {
                    ResourceLayout newLayout = inputIt.second->inputCreation.format != Format::Depth ? ResourceLayout::ShaderReadOnly : ResourceLayout::DepthStencilReadOnly;
                    if (inputIt.second->resource->currentLayout != newLayout)
                    {
                        ResourceBarrierInfo resourceBarrierInfo{};
                        resourceBarrierInfo.texture = inputIt.second->resource->texture;
                        resourceBarrierInfo.oldLayout = inputIt.second->resource->currentLayout;
                        resourceBarrierInfo.newLayout = newLayout;
                        resourceBarrierInfo.isDepth = inputIt.second->inputCreation.format == Format::Depth;
                        cmd.ResourceBarrier(resourceBarrierInfo);

                        inputIt.second->resource->currentLayout = newLayout;
                    }
                }
            }

            for (const auto output : node->outputs)
            {
                if (output.second->creation.type == RenderGraphResourceType::Texture)
                {
                    if (output.second->currentLayout != ResourceLayout::General)
                    {
                        ResourceBarrierInfo resourceBarrierInfo{};
                        resourceBarrierInfo.texture = output.second->texture;
                        resourceBarrierInfo.oldLayout = output.second->currentLayout;
                        resourceBarrierInfo.newLayout = ResourceLayout::General;
                        resourceBarrierInfo.isDepth = false;
                        cmd.ResourceBarrier(resourceBarrierInfo);

                        output.second->currentLayout = ResourceLayout::General;
                    }
                }
            }

            if (node->renderPass)
            {
                BeginRenderPassInfo renderPassInfo{};
                renderPassInfo.renderPass = node->renderPass;

                if (node->clearColor)
                {
                    renderPassInfo.clearValue = &*node->clearColor;
                }

                if (node->clearDepthStencil)
                {
                    renderPassInfo.depthStencil = &*node->clearDepthStencil;
                }

                cmd.BeginRenderPass(renderPassInfo);

                ViewportInfo viewportInfo{};
                viewportInfo.x = 0.;
                viewportInfo.y = 0.;
                viewportInfo.y = (f32) node->extent.height;
                viewportInfo.width = (f32)node->extent.width;
                viewportInfo.height = -(f32)node->extent.height;
                viewportInfo.minDepth = 0.;
                viewportInfo.maxDepth = 1.;

                cmd.SetViewport(viewportInfo);

                auto scissor = Rect{0, 0, node->extent.width, node->extent.height};
                cmd.SetScissor(scissor);
            }

            if (node->renderGraphPass)
            {
                node->renderGraphPass->Render(deltaTime, cmd);
            }

            if (node->renderPass)
            {
                cmd.EndRenderPass();

                for (auto output : node->outputs)
                {
                    if (output.second->creation.type == RenderGraphResourceType::Attachment)
                    {
                        output.second->currentLayout = output.second->textureCreation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
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
    }

    void RenderGraph::BlitSwapchapin(RenderCommands& cmd)
    {
        //TODO
    }

    void RenderGraph::RegisterType(NativeTypeHandler<RenderGraph>& type)
    {

    }
}
