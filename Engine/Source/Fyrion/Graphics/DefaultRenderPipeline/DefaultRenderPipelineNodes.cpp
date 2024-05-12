#include "DefaultRenderPipelineNodes.hpp"

#include "Fyrion/Core/GraphTypes.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "Fyrion/Resource/ResourceGraph.hpp"


namespace Fyrion
{
    void CreateGPUBuffer(const Array<u8>& data, const BufferUsage& usage, Buffer& buffer)
    {
        buffer = Graphics::CreateBuffer(BufferCreation{
            .usage = usage,
            .size = data.Size(),
            .allocation = BufferAllocation::TransferToCPU,
        });

        Graphics::UpdateBufferData(BufferDataInfo{
            .buffer = buffer,
            .data = data.Data(),
            .size = data.Size()
        });
    }

    void DefaultRenderPipelineNodesInit()
    {
        auto geometryRender = Registry::Type<GeometryRender>("Fyrion::GeometryRender");
        geometryRender.Field<&GeometryRender::vertexBuffer>("vertexBuffer").Attribute<GraphInput>();
        geometryRender.Field<&GeometryRender::indexBuffer>("indexBuffer").Attribute<GraphInput>();
        geometryRender.Attribute<ResourceGraphOutput>("Outputs/Geometry Render");


        auto uploadGPUBuffer = Registry::Function<CreateGPUBuffer>("Fyrion::CreateGPUBuffer");
        uploadGPUBuffer.Param<0>("data").Attribute<GraphInput>();
        uploadGPUBuffer.Param<1>("usage").Attribute<GraphInput>();
        uploadGPUBuffer.Param<2>("buffer").Attribute<GraphOutput>();
        uploadGPUBuffer.Attribute<ResourceGraphNode>("Render/Create GPU Buffer");
    }
}
