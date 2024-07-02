#include "Graphics.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "GraphicsTypes.hpp"

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


    void RegisterGraphicsTypes()
    {
        Registry::Type<ShaderStageInfo>();
        Registry::Type<Array<ShaderStageInfo>>();
        Registry::Type<ShaderInfo>();
        Registry::Type<Buffer>();
        Registry::Type<Texture>();

        auto bufferUsage = Registry::Type<BufferUsage>();
        bufferUsage.Value<BufferUsage::VertexBuffer>("VertexBuffer");
        bufferUsage.Value<BufferUsage::IndexBuffer>("IndexBuffer");
        bufferUsage.Value<BufferUsage::UniformBuffer>("UniformBuffer");
        bufferUsage.Value<BufferUsage::StorageBuffer>("StorageBuffer");
        bufferUsage.Value<BufferUsage::IndirectBuffer>("IndirectBuffer");
        bufferUsage.Value<BufferUsage::AccelerationStructureBuild>("AccelerationStructureBuild");
        bufferUsage.Value<BufferUsage::AccelerationStructureStorage>("AccelerationStructureStorage");
        bufferUsage.Value<BufferUsage::All>("All");
    }
}
