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
        if (texture)
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
        if (renderPass)
        {
            Graphics::DestroyRenderPass(renderPass);
        }
    }

    void RenderGraphNode::CreateRenderPass()
    {
        Array<AttachmentCreation> attachments{};
        if (creation.type == RenderGraphPassType::Graphics)
        {
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
                            clearValues.EmplaceBack(output.cleanValue);
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

    RenderGraph::RenderGraph(Extent extent, RenderGraphAsset* asset): asset(asset), viewportExtent(extent)
    {
        Create();
    }

    RenderGraph::~RenderGraph()
    {
        Event::Unbind<OnRecordRenderCommands, &RenderGraph::RecordCommands>(this);

        if (registerSwapchainRenderEvent)
        {
            Event::Unbind<OnSwapchainRender, &RenderGraph::BlitSwapchapin>(this);
        }
    }

    void RenderGraph::Create()
    {
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

        nodes = graph.Sort();

        for (SharedPtr<RenderGraphNode>& node : nodes)
        {
            for (auto& input : node->creation.inputs)
            {
                String inputName = node->name + "#" + input.name;
                for (const auto& edge: asset->GetEdges())
                {
                    if (edge.nodeInput == node->name && edge.input == inputName)
                    {
                        String outputName = edge.nodeOutput + "#" + edge.output;
                        if (auto itResource = resources.Find(outputName))
                        {
                            node->inputs.Insert(input.name, itResource->second);
                        }
                    }
                }
            }

            for(const auto& output: node->creation.outputs)
            {
                if (auto it = node->inputs.Find(output.name))
                {
                    node->outputs.Insert(output.name, it->second);
                    continue;
                }

                String resourceName = node->name + "#" + output.name;
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
                if (resource->texture)
                {
                    Graphics::DestroyTexture(resource->texture);
                }

                Extent size = Extent{viewportExtent.width, viewportExtent.height} * resource->creation.scale;
                resource->textureCreation.extent = {(size.width), (size.width), 1};

                resource->texture = Graphics::CreateTexture(resource->textureCreation);
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
                    resource->textureCreation.extent = {(size.width), (size.width), 1};
                }
                else
                {
                    FY_ASSERT(false, "texture without size");
                }

                resource->textureCreation.format = creation.format;
                if (resource->textureCreation.format != Format::Depth)
                {
                    resource->textureCreation.usage = TextureUsage::RenderPass | TextureUsage::ShaderResource;
                }
                else
                {
                    resource->textureCreation.usage = TextureUsage::DepthStencil | TextureUsage::ShaderResource;
                }

                resource->texture = Graphics::CreateTexture(resource->textureCreation);
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

            //				GPUDevice::BeginLabel(cmd, node.m_name, {node.m_debugColor.x,
            //				                                         node.m_debugColor.y,
            //				                                         node.m_debugColor.z,
            //				                                         1});
            for(const auto& inputIt: node->inputs)
            {
                // if (inputIt.second->creation.type == RenderGraphResourceType::Texture)
                // {
                //     ResourceBarrierInfo resourceBarrierInfo{};
                //     resourceBarrierInfo.texture = inputIt.second->texture;
                //     resourceBarrierInfo.oldLayout = inputIt.second->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                //     resourceBarrierInfo.newLayout = ResourceLayout::ShaderReadOnly;
                //     resourceBarrierInfo.isDepth = inputIt.second->creation.format == Format::Depth;
                //     cmd.ResourceBarrier(resourceBarrierInfo);
                // }
            }

            if (node->renderPass)
            {
                BeginRenderPassInfo renderPassInfo{};
                renderPassInfo.renderPass = node->renderPass;
                renderPassInfo.clearValues = node->clearValues;
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
            }

            cmd.EndLabel();
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
