#include "RenderGraph.hpp"

#include "Graphics.hpp"
#include "Fyrion/Core/Graph.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{

    namespace
    {
	    Logger& logger = Logger::GetLogger("Fyrion::RenderGraph");

	    u32                                      counter{};
	    HashMap<usize, SharedPtr<RenderGraph>>   renderGraphs{};
	    HashMap<String, RenderGraphPassCreation> passes{};
    }

    Texture RenderGraphNode::GetNodeInputTexture(const StringView& inputName)
    {
        auto it = m_inputs.Find(inputName);
        if (it != m_inputs.end())
        {
            return it->second.resource->texture;
        }
        return {nullptr};
    }

    RenderPass RenderGraphNode::GetRenderPass()
    {
        return m_renderPass;
    }

    Extent3D RenderGraphNode::GetRenderPassExtent()
    {
        return m_extent;
    }

    Buffer RenderGraphNode::GetOutputBuffer(const StringView& outputName)
    {
        auto it = m_outputs.Find(outputName);
        if (it != m_outputs.end())
        {
            return it->second->buffer;
        }
        return {};
    }

    Buffer RenderGraphNode::GetInputBuffer(const StringView& outputName)
    {
        auto it = m_inputs.Find(outputName);
        if (it != m_inputs.end())
        {
            return it->second.resource->buffer;
        }
        return {};
    }

    void RenderGraphNode::SetOutputBuffer(const StringView& outputName, const Buffer& buffer)
    {
    	FY_ASSERT(false, "Not implemented");
    }

    void RenderGraphNode::SetOutputReference(const StringView& outputName, VoidPtr reference)
    {
	    if (const auto it = m_outputs.Find(outputName))
    	{
    		it->second->reference = reference;
    	}
    }

    VoidPtr RenderGraphNode::GetInputReference(const StringView& inputName)
    {
	    if (auto it = m_inputs.Find(inputName))
    	{
    		return it->second.resource->reference;
    	}
    	return nullptr;
    }

    RenderGraphNode::RenderGraphNode(const String& name) : m_name(name)
    {
    }

    void RenderGraphNode::CreateRenderPass()
    {
    	if (m_creation.type == RenderGraphPassType::Graphics)
    	{
    		m_clearValues.Clear();
    		Array<AttachmentCreation> attachments{};

    		for(const auto& output: m_creation.outputs)
    		{
    			if (output.type == RenderGraphResourceType::Attachment)
    			{
    				auto& resource = m_outputs[output.name];

    				AttachmentCreation attachmentCreation = AttachmentCreation{
    					.texture        = resource->texture,
						.loadOp         = output.loadOp
					};

    				switch (output.loadOp)
    				{
    					case LoadOp::Load:
    					{
    						attachmentCreation.initialLayout = resource->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
    						attachmentCreation.finalLayout = resource->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
    						break;
    					}
    					case LoadOp::DontCare:
						case LoadOp::Clear:
    					{
    						attachmentCreation.initialLayout = ResourceLayout::Undefined;
    						attachmentCreation.finalLayout = resource->creation.format != Format::Depth ? ResourceLayout::ColorAttachment : ResourceLayout::DepthStencilAttachment;
    						break;
    					}
    				}

    				attachments.EmplaceBack(attachmentCreation);
    				m_extent = resource->extent;

    				if (output.loadOp == LoadOp::Clear)
    				{
    					m_clearValues.EmplaceBack(output.cleanValue);
    				}
    			}
    		}

			// Color color = Color::FromU32(Hash32(node.m_name.CStr()));
			// node.m_debugColor = Vec3{color.FloatRed(), color.FloatGreen(), color.FloatBlue()};

    		RenderPassCreation renderPassCreation{
    			.attachments = attachments
			};

    		m_renderPass = Graphics::CreateRenderPass(renderPassCreation);
    	}
    }

    Extent RenderGraph::GetViewportExtent()
    {
        return m_viewportSize;
    }

    void RenderGraph::Resize(Extent viewportSize)
    {
    }

    void RenderGraph::Destroy()
    {
    	//GPUDevice::WaitGPU();

    	for (auto& node: m_nodes)
    	{

    		if (node->m_renderGraphPass)
    		{
    			node->m_renderGraphPass->Destroy();
    			auto typeHandler = Registry::FindTypeById(node->m_renderPassTypeId);
    			FY_ASSERT(typeHandler, "type not found");
    			typeHandler->Destroy(node->m_renderGraphPass);
    		}

    		if (node->m_renderPass)
    		{
    			Graphics::DestroyRenderPass(node->m_renderPass);
    		}
    	}

    	for(auto& resource : m_resources)
    	{
    		DestroyResource(*resource.second);
    	}

	    renderGraphs.Erase(renderGraphs.Find(m_id));
    }

    RenderGraph* RenderGraph::Create(const Extent& viewportSize, RID renderGraphId)
    {
		u32 id = counter++;
		SharedPtr<RenderGraph> renderGraph = MakeShared<RenderGraph>(id, viewportSize);
		renderGraphs.Insert(Pair<usize, SharedPtr<RenderGraph>>(id, renderGraph));

		ResourceObject asset = Repository::Read(renderGraphId);
		FY_ASSERT(asset, "Render Graph Asset not found");

		Graph<String, SharedPtr<RenderGraphNode>> graph{};

		HashMap<String, HashMap<String, String>> edges{};

    	Span<String> assetPasses = asset[RenderGraphAsset::Passes].As<Span<String>>();
    	Span<RenderGraphEdge> assetEdges = asset[RenderGraphAsset::Passes].As<Span<RenderGraphEdge>>();

		for(const auto& pass: assetPasses)
		{
			graph.AddNode(pass, MakeShared<RenderGraphNode>(pass));
		}

		//origin become dest.
		for (const auto& edge: assetEdges)
		{
			auto destNode = WithoutLast(edge.dest, "::");
			auto destEdge = Last(edge.dest, "::");
			auto withoutLast = WithoutLast(edge.origin, "::");
			graph.AddEdge(destNode, withoutLast);
			logger.Debug("added graph edge {} {}", destNode, withoutLast);

			auto it = edges.Find(destNode);
			if (it == edges.end())
			{
				it = edges.Insert(destNode, HashMap<String, String>()).first;
			}
			it->second.Insert(MakePair(String{destEdge}, String{edge.origin}));
			logger.Debug("added edge {} {} {}", destNode, destEdge, edge.origin);
		}

		renderGraph->m_nodes = graph.Sort();
		for (auto& node: renderGraph->m_nodes)
		{
			logger.Debug("sort: node {} ", node->m_name);
		}

		for (auto& node: renderGraph->m_nodes)
		{
			HashMap<String, String>& nodeEdges = edges[node->m_name];

			auto dbIt = passes.Find(node->m_name);
			FY_ASSERT(dbIt != passes.end(), "node not found");
			node->m_creation = dbIt->second;

			//dest = input
			for(auto& input: node->m_creation.inputs)
			{
				auto it = nodeEdges.Find(input.name);
				if (it != nodeEdges.end())
				{
					auto itRes = renderGraph->m_resources.Find(it->second);
					FY_ASSERT(itRes != renderGraph->m_resources.end(), "resource not found");
					FY_ASSERT(itRes->second->creation.format == input.format, "different formats");
					node->m_inputs.Insert(MakePair(input.name, InputNodeResource{input, itRes->second}));
					logger.Debug("added input {}, {} ", node->m_name, input.name);
				}
			}

			//origin = output
			for(const auto& output: node->m_creation.outputs)
			{
				String resourceName = node->m_name + "::" + output.name;
				auto inputIt = node->m_inputs.Find(output.name);
				if (inputIt != node->m_inputs.end())
				{
					node->m_outputs.Insert(MakePair(String{output.name}, inputIt->second.resource));
					renderGraph->m_resources.Insert(MakePair(resourceName, inputIt->second.resource));
				}
				else
				{
					auto resource = MakeShared<RenderGraphResource>();
					renderGraph->CreateResource(output, *renderGraph->m_resources.Insert(MakePair(resourceName, resource)).first->second);
					node->m_outputs.Insert(MakePair(String{output.name}, resource));
					logger.Debug("Created resource {} ", resourceName);
				}
			}

			node->CreateRenderPass();

			logger.Debug("creating node {} ", node->m_name);

			TypeHandler* typeHandler = Registry::FindTypeByName(node->m_name);
			if (typeHandler)
			{
				for (TypeID baseType : typeHandler->GetBaseTypes())
				{
					if (baseType == GetTypeID<RenderGraphPass>())
					{
						RenderGraphPass* renderGraphPass = typeHandler->Cast<RenderGraphPass>(typeHandler->NewInstance());
						renderGraphPass->node = node.Get();
						node->m_renderGraphPass = renderGraphPass;
						node->m_renderPassTypeId = typeHandler->GetTypeInfo().typeId;
					}
				}
			}

			if (node->m_renderGraphPass)
			{
				node->m_renderGraphPass->Init();
			}
		}

		return renderGraph.Get();
    }

    void RenderGraph::RegisterPass(const RenderGraphPassCreation& renderGraphPassCreation)
    {
        logger.Debug("{} Registered", renderGraphPassCreation.name);
        passes.Emplace(renderGraphPassCreation.name, RenderGraphPassCreation{renderGraphPassCreation});
    }

    Texture RenderGraph::GetColorOutput() const
    {
        return {};
    }

    Texture RenderGraph::GetDepthOutput() const
    {
        return {};
    }

    RenderGraph::RenderGraph(u32 id, const Extent& viewportSize) : m_id(id), m_viewportSize(viewportSize)
    {
    }

    void RenderGraph::CreateResource(const RenderGraphResourceCreation& renderGraphResourceCreation, RenderGraphResource& renderGraphResource) const
    {
        renderGraphResource.creation = renderGraphResourceCreation;

        switch (renderGraphResourceCreation.type)
        {
            case RenderGraphResourceType::Buffer:
            {
                if (renderGraphResourceCreation.bufferInitialSize > 0)
                {
                    renderGraphResource.buffer = Graphics::CreateBuffer(BufferCreation{
                        .usage = renderGraphResourceCreation.bufferUsage,
                        .size = renderGraphResourceCreation.bufferInitialSize,
                        .memory = renderGraphResourceCreation.bufferMemory
                    });
                    renderGraphResource.ownResource = true;
                    renderGraphResource.destroyed = false;
                }
                break;
            }
            case RenderGraphResourceType::Texture:
            case RenderGraphResourceType::Attachment:
            {
                TextureCreation textureCreation{};
                if (renderGraphResourceCreation.size > 0)
                {
                    textureCreation.extent = renderGraphResourceCreation.size;
                }
                else if (renderGraphResourceCreation.scale > 0.f)
                {
                    Extent size = Extent{m_viewportSize.width, m_viewportSize.height} * renderGraphResourceCreation.scale;
                    textureCreation.extent = {(size.width), (size.width), 1};
                }
                else
                {
                    FY_ASSERT(false, "texture without size");
                }

                renderGraphResource.extent = textureCreation.extent;

                textureCreation.format = renderGraphResourceCreation.format;
                if (textureCreation.format != Format::Depth)
                {
                    textureCreation.usage = TextureUsage::RenderPass | TextureUsage::ShaderResource;
                }
                else
                {
                    textureCreation.usage = TextureUsage::DepthStencil | TextureUsage::ShaderResource;
                }

                renderGraphResource.texture = Graphics::CreateTexture(textureCreation);
                renderGraphResource.ownResource = true;
                renderGraphResource.destroyed = false;

                break;
            }
            case RenderGraphResourceType::Reference:
            {
                break;
            }
        }
    }

    void RenderGraph::DestroyResource(RenderGraphResource& renderGraphResource)
    {
        if (renderGraphResource.destroyed) return;
        if (!renderGraphResource.ownResource) return;

        renderGraphResource.destroyed = true;

        switch (renderGraphResource.creation.type)
        {
            case RenderGraphResourceType::Buffer:
            {
                Graphics::DestroyBuffer(renderGraphResource.buffer);
                break;
            }
            case RenderGraphResourceType::Texture:
            case RenderGraphResourceType::Attachment:
            {
                Graphics::DestroyTexture(renderGraphResource.texture);
                break;
            }
            case RenderGraphResourceType::Reference:
            {
                break;
            }
        }
    }
}
