#pragma once
#include "GraphicsTypes.hpp"


namespace Fyrion
{
    class EquirectangularToCubemap
    {
    public:
        void Init(Extent extent, Format format);
        void Destroy();
        void Convert(RenderCommands& cmd, Texture originTexture);

        Texture GetTexture() const;

    private:
        Format        format{};
        Extent        extent{};
        Texture       texture = {};
        TextureView   textureArrayView = {};
        PipelineState pipelineState = {};
        BindingSet*   bindingSet = nullptr;
    };

    class DiffuseIrradianceGenerator
    {
    public:
        void Init(Extent extent);
        void Destroy();
        void Generate(RenderCommands& cmd, Texture cubemap);
        Texture GetTexture() const;

    private:
        Extent        extent{};
        Texture       texture{};
        TextureView   textureArrayView{};
        PipelineState pipelineState{};
        BindingSet*   bindingSet{};
    };
}

namespace Fyrion::RenderUtils
{
    FY_API AABB    CalculateMeshAABB(const Array<VertexStride>& vertices);
    FY_API void    CalcTangents(Array<VertexStride>& vertices, const Array<u32>& indices, bool useMikktspace = true);
    FY_API Texture GenerateBRDFLUT();
}
