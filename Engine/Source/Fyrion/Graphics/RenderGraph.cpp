#include "RenderGraph.hpp"

#include "Graphics.hpp"
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Graph.hpp"
#include "Fyrion/Core/Logger.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::RenderGraph", LogLevel::Debug);
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
                if (output->creation.type == RenderGraphResourceType::Attachment)
                {
                    AttachmentCreation attachmentCreation = AttachmentCreation{
                        .texture = output->texture,
                        .loadOp = loadOp
                    };

                    switch (loadOp)
                    {
                        case LoadOp::Load:
                        {
                            attachmentCreation.initialLayout = output->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                            attachmentCreation.finalLayout = output->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                            break;
                        }
                        case LoadOp::DontCare:
                        case LoadOp::Clear:
                        {
                            attachmentCreation.initialLayout = ResourceLayout::Undefined;
                            attachmentCreation.finalLayout = output->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
                            break;
                        }
                    }

                    attachments.EmplaceBack(attachmentCreation);

                    //extent = it->second->textureCreation.extent;
                }
            }

            RenderPassCreation renderPassCreation{
                .attachments = attachments
            };

            renderPass = Graphics::CreateRenderPass(renderPassCreation);
        }
    }

    RenderPassBuilder::RenderPassBuilder(RenderGraphPass* pass) : pass(pass) {}

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

    RenderPassBuilder& RenderPassBuilder::Init(const std::function<void(RenderGraphPass& pass)>& func)
    {
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::Update(const std::function<void(RenderGraphPass& pass)>& func)
    {
        return *this;
    }

    RenderPassBuilder& RenderPassBuilder::Render(const std::function<void(RenderGraphPass& pass, RenderCommands& cmd)>& func)
    {
        return *this;
    }

    RenderPassBuilder RenderGraph::AddPass(StringView name, RenderGraphPassType type)
    {
        SharedPtr<RenderGraphPass> pass = MakeShared<RenderGraphPass>();
        pass->id = passes.Size() + 1;
        pass->name = name;
        pass->type = type;
        passes.EmplaceBack(pass);
        return {pass.Get()};
    }

    RenderGraphResource* RenderGraph::Create(const RenderGraphResourceCreation& creation)
    {
        SharedPtr<RenderGraphResource> resource = MakeShared<RenderGraphResource>(creation);
        resources.EmplaceBack(resource);
        return resource.Get();
    }

    void RenderGraph::Bake(Extent viewportExtent)
    {
        this->viewportExtent = viewportExtent;

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
            logger.Debug("pass {} created ", pass->name);
        }

        Event::Bind<OnRecordRenderCommands, &RenderGraph::RecordCommands>(this);
    }

    void RenderGraph::RecordCommands(RenderCommands& cmd, f64 deltaTime)
    {

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
