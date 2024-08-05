#include "RenderUtils.hpp"

#include <algorithm>
#include <mikktspace.h>

#include "Graphics.hpp"
#include "Fyrion/Asset/AssetDatabase.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Graphics/Assets/ShaderAsset.hpp"


namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::RenderUtils");

        struct SpecularMapFilterSettings
        {
            alignas(16) f32 roughness;
        };

        struct UserData
        {
            Array<VertexStride>& vertices;
            const Array<u32>&    indices;
        };


        i32 GetNumFaces(const SMikkTSpaceContext* pContext)
        {
            UserData& mesh = *static_cast<UserData*>(pContext->m_pUserData);
            return (i32)mesh.indices.Size() / 3;
        }

        i32 GetNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace)
        {
            return 3;
        }

        u32 GetVertexIndex(const SMikkTSpaceContext* context, i32 iFace, i32 iVert)
        {
            UserData& mesh = *static_cast<UserData*>(context->m_pUserData);
            auto      faceSize = GetNumVerticesOfFace(context, iFace);
            auto      indicesIndex = (iFace * faceSize) + iVert;
            return mesh.indices[indicesIndex];
        }

        void GetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
        {
            UserData& mesh = *static_cast<UserData*>(pContext->m_pUserData);

            VertexStride& v = mesh.vertices[GetVertexIndex(pContext, iFace, iVert)];
            fvPosOut[0] = v.position.x;
            fvPosOut[1] = v.position.y;
            fvPosOut[2] = v.position.z;
        }

        void GetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
        {
            UserData&     mesh = *static_cast<UserData*>(pContext->m_pUserData);
            VertexStride& v = mesh.vertices[GetVertexIndex(pContext, iFace, iVert)];
            fvNormOut[0] = v.normal.x;
            fvNormOut[1] = v.normal.y;
            fvNormOut[2] = v.normal.z;
        }

        void GetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
        {
            UserData&     mesh = *static_cast<UserData*>(pContext->m_pUserData);
            VertexStride& v = mesh.vertices[GetVertexIndex(pContext, iFace, iVert)];
            fvTexcOut[0] = v.uv.x;
            fvTexcOut[1] = v.uv.y;
        }

        void SetTangentSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
        {
            UserData&     mesh = *static_cast<UserData*>(pContext->m_pUserData);
            VertexStride& v = mesh.vertices[GetVertexIndex(pContext, iFace, iVert)];
            v.tangent.x = fvTangent[0];
            v.tangent.y = fvTangent[1];
            v.tangent.z = fvTangent[2];
            v.tangent.w = -fSign;
        }

        Vec3 CalculateTangent(const VertexStride& v1, const VertexStride& v2, const VertexStride& v3)
        {
            Vec3 edge1 = v2.position - v1.position;
            Vec3 edge2 = v3.position - v1.position;
            Vec2 deltaUV1 = v2.uv - v1.uv;
            Vec2 deltaUV2 = v3.uv - v1.uv;

            Vec3 tangent{};

            float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
            tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
            tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
            tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

            return tangent;
        }

        void CalculateTangents(Array<VertexStride>& vertices, const Array<u32>& indices)
        {
            //Calculate tangents
            for (usize i = 0; i < indices.Size(); i += 3)
            {
                u32 idx0 = indices[i + 0];
                u32 idx1 = indices[i + 1];
                u32 idx2 = indices[i + 2];

                vertices[idx0].tangent = Vec4{CalculateTangent(vertices[idx0], vertices[idx1], vertices[idx2]), 1.0};
                vertices[idx1].tangent = Vec4{CalculateTangent(vertices[idx1], vertices[idx2], vertices[idx0]), 1.0};
                vertices[idx2].tangent = Vec4{CalculateTangent(vertices[idx2], vertices[idx0], vertices[idx1]), 1.0};
            }
        }


        void CalculateTangents(Array<VertexStride>& vertices)
        {
            //Calculate tangents
            for (usize i = 0; i < vertices.Size(); i += 3)
            {
                u32 idx0 = i + 0;
                u32 idx1 = i + 1;
                u32 idx2 = i + 2;
                vertices[idx0].tangent = Vec4{CalculateTangent(vertices[idx0], vertices[idx1], vertices[idx2]), 1.0};
                vertices[idx1].tangent = Vec4{CalculateTangent(vertices[idx1], vertices[idx2], vertices[idx0]), 1.0};
                vertices[idx2].tangent = Vec4{CalculateTangent(vertices[idx2], vertices[idx0], vertices[idx1]), 1.0};
            }
        }
    }


    AABB RenderUtils::CalculateMeshAABB(const Array<VertexStride>& vertices)
    {
        AABB boundingBox{};

        if (!vertices.Empty())
        {
            boundingBox.min = vertices[0].position;
            boundingBox.max = vertices[0].position;

            for (const auto& dataIt : vertices)
            {
                boundingBox.min = Math::Min(boundingBox.min, dataIt.position);
                boundingBox.max = Math::Max(boundingBox.max, dataIt.position);
            }
        }
        return boundingBox;
    }

    void RenderUtils::CalcTangents(Array<VertexStride>& vertices, const Array<u32>& indices, bool useMikktspace)
    {
        if (useMikktspace)
        {
            UserData userData{
                .vertices = vertices,
                .indices = indices
            };

            SMikkTSpaceInterface anInterface{
                .m_getNumFaces = GetNumFaces,
                .m_getNumVerticesOfFace = GetNumVerticesOfFace,
                .m_getPosition = GetPosition,
                .m_getNormal = GetNormal,
                .m_getTexCoord = GetTexCoord,
                .m_setTSpaceBasic = SetTangentSpaceBasic
            };

            SMikkTSpaceContext mikkTSpaceContext{
                .m_pInterface = &anInterface,
                .m_pUserData = &userData
            };

            genTangSpaceDefault(&mikkTSpaceContext);
        }
        else
        {
            CalculateTangents(vertices, indices);
        }
    }

    Texture RenderUtils::GenerateBRDFLUT()
    {
        return {};
    }

    void EquirectangularToCubemap::Init(Extent extent, Format format)
    {
        this->format = format;
        this->extent = extent;

        texture = Graphics::CreateTexture(TextureCreation{
            .extent = {extent.width, extent.height, 1},
            .format = format,
            .usage = TextureUsage::Storage | TextureUsage::ShaderResource,
            .arrayLayers = 6,
            .name = "Cubemap"
        });

        textureArrayView = Graphics::CreateTextureView(TextureViewCreation{
            .texture = texture,
            .viewType = ViewType::Type2DArray,
            .layerCount = 6,
        });

        ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Utils/EquirectToCube.comp");

        pipelineState = Graphics::CreateComputePipelineState({
            .shader = shaderAsset
        });

        bindingSet = Graphics::CreateBindingSet(shaderAsset);
    }

    void EquirectangularToCubemap::Destroy()
    {
        Graphics::DestroyBindingSet(bindingSet);
        Graphics::DestroyComputePipelineState(pipelineState);
        Graphics::DestroyTextureView(textureArrayView);
        Graphics::DestroyTexture(texture);
    }

    void EquirectangularToCubemap::Convert(RenderCommands& cmd, Texture originTexture)
    {
        bindingSet->GetVar("inputTexture")->SetTexture(originTexture);
        bindingSet->GetVar("outputTexture")->SetTextureView(textureArrayView);

        cmd.ResourceBarrier(ResourceBarrierInfo{
            .texture = texture,
            .oldLayout = ResourceLayout::Undefined,
            .newLayout = ResourceLayout::General,
            .layerCount = 6
        });

        cmd.BindPipelineState(pipelineState);
        cmd.BindBindingSet(pipelineState, bindingSet);

        cmd.Dispatch(std::ceil(extent.width / 32.f),
                     std::ceil(extent.height / 32.f),
                     6.0f);

        cmd.ResourceBarrier(ResourceBarrierInfo{
            .texture = texture,
            .oldLayout = ResourceLayout::General,
            .newLayout = ResourceLayout::ShaderReadOnly,
            .layerCount = 6
        });
    }

    Texture EquirectangularToCubemap::GetTexture() const
    {
        return texture;
    }

    void DiffuseIrradianceGenerator::Init(Extent extent)
    {
        this->extent = extent;

        texture = Graphics::CreateTexture(TextureCreation{
            .extent = {extent.width, extent.height, 1},
            .format = Format::RGBA16F,
            .usage = TextureUsage::Storage | TextureUsage::ShaderResource,
            .arrayLayers = 6,
            .name = "Irradiance"
        });

        Graphics::UpdateTextureLayout(texture, ResourceLayout::Undefined, ResourceLayout::ShaderReadOnly);

        textureArrayView = Graphics::CreateTextureView(TextureViewCreation{
            .texture = texture,
            .viewType = ViewType::Type2DArray,
            .layerCount = 6,
        });

        ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Utils/IRMap.comp");

        pipelineState = Graphics::CreateComputePipelineState({
            .shader = shaderAsset
        });

        bindingSet = Graphics::CreateBindingSet(shaderAsset);
    }

    void DiffuseIrradianceGenerator::Generate(RenderCommands& cmd, Texture cubemap)
    {
        bindingSet->GetVar("inputTexture")->SetTexture(cubemap);
        bindingSet->GetVar("outputTexture")->SetTextureView(textureArrayView);

        cmd.ResourceBarrier(ResourceBarrierInfo{
            .texture = texture,
            .oldLayout = ResourceLayout::ShaderReadOnly,
            .newLayout = ResourceLayout::General,
            .layerCount = 6
        });

        cmd.BindPipelineState(pipelineState);
        cmd.BindBindingSet(pipelineState, bindingSet);

        cmd.Dispatch(std::ceil(extent.width / 32.f),
                     std::ceil(extent.height / 32.f),
                     6.0f);

        cmd.ResourceBarrier(ResourceBarrierInfo{
            .texture = texture,
            .oldLayout = ResourceLayout::General,
            .newLayout = ResourceLayout::ShaderReadOnly,
            .layerCount = 6
        });
    }

    Texture DiffuseIrradianceGenerator::GetTexture() const
    {
        return texture;
    }

    void BRDFLUTGenerator::Init(Extent extent)
    {
        texture = Graphics::CreateTexture(TextureCreation{
            .extent = {extent.width, extent.height, 1},
            .format = Format::RG16F,
            .usage = TextureUsage::Storage | TextureUsage::ShaderResource,
            .arrayLayers = 1,
            .name = "BRDFLUT"
        });

        sampler = Graphics::CreateSampler(SamplerCreation{
            .addressMode = TextureAddressMode::ClampToEdge,
        });

        ShaderAsset* shader = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Utils/GenBRDFLUT.comp");

        PipelineState pipelineState = Graphics::CreateComputePipelineState({
            .shader = shader
        });

        BindingSet* bindingSet = Graphics::CreateBindingSet(shader);
        bindingSet->GetVar("texture")->SetTexture(texture);

        RenderCommands& cmd = Graphics::GetCmd();
        cmd.Begin();

        cmd.ResourceBarrier(ResourceBarrierInfo{
            .texture = texture,
            .oldLayout = ResourceLayout::Undefined,
            .newLayout = ResourceLayout::General,
        });

        cmd.BindPipelineState(pipelineState);
        cmd.BindBindingSet(pipelineState, bindingSet);
        cmd.Dispatch(extent.width / 32.f,
                     extent.height / 32.f,
                     1);

        cmd.ResourceBarrier(ResourceBarrierInfo{
            .texture = texture,
            .oldLayout = ResourceLayout::General,
            .newLayout = ResourceLayout::ShaderReadOnly,
        });

        cmd.SubmitAndWait(Graphics::GetMainQueue());

        Graphics::DestroyComputePipelineState(pipelineState);
        Graphics::DestroyBindingSet(bindingSet);
    }

    void BRDFLUTGenerator::Destroy()
    {
        Graphics::DestroyTexture(texture);
        Graphics::DestroySampler(sampler);
    }

    Texture BRDFLUTGenerator::GetTexture() const
    {
        return texture;
    }

    Sampler BRDFLUTGenerator::GetSampler() const
    {
        return sampler;
    }

    void SpecularMapGenerator::Init(Extent extent, u32 mips)
    {
        this->extent = extent;
        this->mips = mips;

        texture = Graphics::CreateTexture(TextureCreation{
            .extent = {extent.width, extent.height, 1},
            .format = Format::RGBA16F,
            .usage = TextureUsage::Storage | TextureUsage::ShaderResource,
            .mipLevels = mips,
            .arrayLayers = 6,
            .name = "SpecularMap"
        });

        Graphics::UpdateTextureLayout(texture, ResourceLayout::Undefined, ResourceLayout::ShaderReadOnly);

        ShaderAsset* shaderAsset = AssetDatabase::FindByPath<ShaderAsset>("Fyrion://Shaders/Utils/SpecularMap.comp");

        pipelineState = Graphics::CreateComputePipelineState({
            .shader = shaderAsset
        });

        bindingSets.Resize(mips);
        textureViews.Resize(mips);

        for (u32 i = 0; i < mips; ++i)
        {
            bindingSets[i] = Graphics::CreateBindingSet(shaderAsset);
            textureViews[i] = Graphics::CreateTextureView(TextureViewCreation{
                .texture = texture,
                .viewType = ViewType::Type2DArray,
                .baseMipLevel = i,
                .layerCount = 6,
            });
        }
    }

    void SpecularMapGenerator::Generate(RenderCommands& cmd, Texture cubemap)
    {
        for (int i = 0; i < mips; ++i)
        {
            bindingSets[i]->GetVar("inputTexture")->SetTexture(cubemap);
            bindingSets[i]->GetVar("outputTexture")->SetTextureView(textureViews[i]);
        }

        cmd.BindPipelineState(pipelineState);

        for (u32 mip = 0; mip < mips; ++mip)
        {
            cmd.ResourceBarrier(ResourceBarrierInfo{
                .texture = texture,
                .oldLayout = ResourceLayout::ShaderReadOnly,
                .newLayout = ResourceLayout::General,
                .mipLevel = mip,
                .layerCount = 6
            });

            u32 mipWidth  = extent.width * std::pow(0.5, mip);
            u32 mipHeight = extent.height * std::pow(0.5, mip);

            SpecularMapFilterSettings settings{
                .roughness = static_cast<float>(mip) / static_cast<float>(mips - 1)
            };

            cmd.PushConstants(pipelineState, ShaderStage::Compute, &settings, sizeof(settings));
            cmd.BindBindingSet(pipelineState, bindingSets[mip]);
            cmd.Dispatch(std::ceil(mipWidth / 32.f),
                         std::ceil(mipHeight / 32.f),
                         6);

            cmd.ResourceBarrier(ResourceBarrierInfo{
                .texture = texture,
                .oldLayout = ResourceLayout::General,
                .newLayout = ResourceLayout::ShaderReadOnly,
                .mipLevel = mip,
                .layerCount = 6
            });
        }
    }

    Texture SpecularMapGenerator::GetTexture() const
    {
        return texture;
    }

    void SpecularMapGenerator::Destroy()
    {
        Graphics::DestroyTexture(texture);
        Graphics::DestroyComputePipelineState(pipelineState);

        for (u32 i = 0; i < mips; ++i)
        {
            Graphics::DestroyBindingSet(bindingSets[i]);
            Graphics::DestroyTextureView(textureViews[i]);
        }
    }

    void DiffuseIrradianceGenerator::Destroy()
    {
        Graphics::DestroyComputePipelineState(pipelineState);
        Graphics::DestroyTexture(texture);
        Graphics::DestroyTextureView(textureArrayView);
        Graphics::DestroyBindingSet(bindingSet);
    }
}
