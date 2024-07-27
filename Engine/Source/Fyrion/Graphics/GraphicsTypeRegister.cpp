#include "Graphics.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "GraphicsTypes.hpp"
#include "RenderGraph.hpp"
#include "Assets/DCCAsset.hpp"
#include "Assets/ShaderAsset.hpp"
#include "Assets/TextureAsset.hpp"

namespace Fyrion
{
    template<>
    struct ReleaseHandler<Buffer>
    {
        static void Release(const Buffer& value)
        {
            Graphics::DestroyBuffer(value);
        }
    };

    template<>
    struct ReleaseHandler<Texture>
    {
        static void Release(const Texture& value)
        {
            Graphics::DestroyTexture(value);
        }
    };

    void RegisterShaderIO();
    void RegisterTextureIO();
    void RegisterGLTFIO();

    void RegisterGraphicsTypes()
    {
        Registry::Type<ShaderStageInfo>();
        Registry::Type<Array<ShaderStageInfo>>();
        Registry::Type<ShaderInfo>();
        Registry::Type<Buffer>();
        Registry::Type<Texture>();
        Registry::Type<RenderGraph>();
        Registry::Type<RenderGraphEdge>();
        Registry::Type<RenderGraphAsset>();
        Registry::Type<RenderGraphPass>();
        Registry::Type<ShaderAsset>();
        Registry::Type<TextureAsset>();
        Registry::Type<DCCAsset>();
        Registry::Type<MeshAsset>();
        Registry::Type<MaterialAsset>();
        Registry::Type<MeshPrimitive>();

        RegisterGLTFIO();
        RegisterShaderIO();
        RegisterTextureIO();

        auto bufferUsage = Registry::Type<BufferUsage>();
        bufferUsage.Value<BufferUsage::VertexBuffer>("VertexBuffer");
        bufferUsage.Value<BufferUsage::IndexBuffer>("IndexBuffer");
        bufferUsage.Value<BufferUsage::UniformBuffer>("UniformBuffer");
        bufferUsage.Value<BufferUsage::StorageBuffer>("StorageBuffer");
        bufferUsage.Value<BufferUsage::IndirectBuffer>("IndirectBuffer");
        bufferUsage.Value<BufferUsage::AccelerationStructureBuild>("AccelerationStructureBuild");
        bufferUsage.Value<BufferUsage::AccelerationStructureStorage>("AccelerationStructureStorage");
        bufferUsage.Value<BufferUsage::All>("All");

        auto shaderAssetType = Registry::Type<ShaderAssetType>();
        shaderAssetType.Value<ShaderAssetType::None>("None");
        shaderAssetType.Value<ShaderAssetType::Graphics>("Graphics");
        shaderAssetType.Value<ShaderAssetType::Compute>("Compute");
        shaderAssetType.Value<ShaderAssetType::Raytrace>("Raytrace");


        auto alphaMode = Registry::Type<AlphaMode>();
        alphaMode.Value<AlphaMode::None>("None");
        alphaMode.Value<AlphaMode::Opaque>("Opaque");
        alphaMode.Value<AlphaMode::Mask>("Mask");
        alphaMode.Value<AlphaMode::Blend>("Blend");
    }
}
