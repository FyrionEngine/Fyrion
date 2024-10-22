#include "Graphics.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "GraphicsTypes.hpp"
#include "RenderPipeline.hpp"
#include "Assets/MeshAsset.hpp"
#include "Assets/TextureAsset.hpp"
#include "Assets/ShaderAsset.hpp"

namespace Fyrion
{

    void RegisterDefaultRenderPipeline();

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

    void RegisterGraphicsTypes()
    {
        Registry::Type<InterfaceVariable>();
        Registry::Type<TypeDescription>();
        Registry::Type<DescriptorBinding>();
        Registry::Type<DescriptorLayout>();
        Registry::Type<ShaderPushConstant>();
        Registry::Type<ShaderStageInfo>();
        Registry::Type<ShaderInfo>();
        Registry::Type<Buffer>();
        Registry::Type<Texture>();

        Registry::Type<RenderPipeline>();


        Registry::Type<ShaderAsset>();
        Registry::Type<MeshAsset>();
        Registry::Type<MeshPrimitive>();
        Registry::Type<MaterialAsset>();

        Registry::Type<TextureAsset>();

        auto bufferUsage = Registry::Type<BufferUsage>();
        bufferUsage.Value<BufferUsage::VertexBuffer>("VertexBuffer");
        bufferUsage.Value<BufferUsage::IndexBuffer>("IndexBuffer");
        bufferUsage.Value<BufferUsage::UniformBuffer>("UniformBuffer");
        bufferUsage.Value<BufferUsage::StorageBuffer>("StorageBuffer");
        bufferUsage.Value<BufferUsage::IndirectBuffer>("IndirectBuffer");
        bufferUsage.Value<BufferUsage::AccelerationStructureBuild>("AccelerationStructureBuild");
        bufferUsage.Value<BufferUsage::AccelerationStructureStorage>("AccelerationStructureStorage");
        bufferUsage.Value<BufferUsage::All>("All");

        auto alphaMode = Registry::Type<AlphaMode>();
        alphaMode.Value<AlphaMode::None>("None");
        alphaMode.Value<AlphaMode::Opaque>("Opaque");
        alphaMode.Value<AlphaMode::Mask>("Mask");
        alphaMode.Value<AlphaMode::Blend>("Blend");

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

        auto lightType = Registry::Type<LightType>();
        lightType.Value<LightType::Directional>("Directional");
        lightType.Value<LightType::Point>("Point");
        lightType.Value<LightType::Spot>("Spot");
        lightType.Value<LightType::Area>("Area");

        auto shaderStage = Registry::Type<ShaderStage>();
        shaderStage.Value<ShaderStage::Unknown>("Unknown");
        shaderStage.Value<ShaderStage::Vertex>("Vertex");
        shaderStage.Value<ShaderStage::Hull>("Hull");
        shaderStage.Value<ShaderStage::Domain>("Domain");
        shaderStage.Value<ShaderStage::Geometry>("Geometry");
        shaderStage.Value<ShaderStage::Pixel>("Pixel");
        shaderStage.Value<ShaderStage::Compute>("Compute");
        shaderStage.Value<ShaderStage::Amplification>("Amplification");
        shaderStage.Value<ShaderStage::Mesh>("Mesh");
        shaderStage.Value<ShaderStage::RayGen>("RayGen");
        shaderStage.Value<ShaderStage::RayMiss>("RayMiss");
        shaderStage.Value<ShaderStage::RayClosestHit>("RayClosestHit");
        shaderStage.Value<ShaderStage::RayAnyHit>("RayAnyHit");
        shaderStage.Value<ShaderStage::RayIntersection>("RayIntersection");
        shaderStage.Value<ShaderStage::Callable>("Callable");
        shaderStage.Value<ShaderStage::All>("All");

        auto descriptorType = Registry::Type<DescriptorType>();
        descriptorType.Value<DescriptorType::SampledImage>("SampledImage");
        descriptorType.Value<DescriptorType::Sampler>("Sampler");
        descriptorType.Value<DescriptorType::StorageImage>("StorageImage");
        descriptorType.Value<DescriptorType::UniformBuffer>("UniformBuffer");
        descriptorType.Value<DescriptorType::StorageBuffer>("StorageBuffer");
        descriptorType.Value<DescriptorType::AccelerationStructure>("AccelerationStructure");

        auto renderType = Registry::Type<RenderType>();
        renderType.Value<RenderType::None>("None");
        renderType.Value<RenderType::Void>("Void");
        renderType.Value<RenderType::Bool>("Bool");
        renderType.Value<RenderType::Int>("Int");
        renderType.Value<RenderType::Float>("Float");
        renderType.Value<RenderType::Vector>("Vector");
        renderType.Value<RenderType::Matrix>("Matrix");
        renderType.Value<RenderType::Image>("Image");
        renderType.Value<RenderType::Sampler>("Sampler");
        renderType.Value<RenderType::SampledImage>("SampledImage");
        renderType.Value<RenderType::Array>("Array");
        renderType.Value<RenderType::RuntimeArray>("RuntimeArray");
        renderType.Value<RenderType::Struct>("Struct");

        auto viewType = Registry::Type<ViewType>();
        viewType.Value<ViewType::Type1D>("Type1D");
        viewType.Value<ViewType::Type2D>("Type2D");
        viewType.Value<ViewType::Type3D>("Type3D");
        viewType.Value<ViewType::TypeCube>("TypeCube");
        viewType.Value<ViewType::Type1DArray>("Type1DArray");
        viewType.Value<ViewType::Type2DArray>("Type2DArray");
        viewType.Value<ViewType::TypeCubeArray>("TypeCubeArray");
        viewType.Value<ViewType::Undefined>("Undefined");

        RegisterDefaultRenderPipeline();
    }
}
