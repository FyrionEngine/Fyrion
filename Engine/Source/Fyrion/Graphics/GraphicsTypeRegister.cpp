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
        Registry::Type<TextureAssetImage>();

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

        auto textureType = Registry::Type<TextureType>();
        textureType.Value<TextureType::Texture2D>("Texture2D");
        textureType.Value<TextureType::Texture3D>("Texture3D");
        textureType.Value<TextureType::Cubemap>("Cubemap");

        auto format = Registry::Type<Format>();
        format.Value<Format::R>("R");
        format.Value<Format::R16F>("R16F");
        format.Value<Format::R32F>("R32F");
        format.Value<Format::RG>("RG");
        format.Value<Format::RG16F>("RG16F");
        format.Value<Format::RG32F>("RG32F");
        format.Value<Format::RGB>("RGB");
        format.Value<Format::RGB16F>("RGB16F");
        format.Value<Format::RGB32F>("RGB32F");
        format.Value<Format::RGBA>("RGBA");
        format.Value<Format::RGBA16F>("RGBA16F");
        format.Value<Format::RGBA32F>("RGBA32F");
        format.Value<Format::BGRA>("BGRA");
        format.Value<Format::Depth>("Depth");
        format.Value<Format::Undefined>("Undefined");
    }
}
