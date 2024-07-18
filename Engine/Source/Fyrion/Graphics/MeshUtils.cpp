#include "MeshUtils.hpp"
#include <mikktspace.h>


namespace Fyrion
{
    namespace
    {
        struct UserData
        {
            Array<VertexStride>& vertices;
            const Array<u32>&  indices;
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
            UserData&   mesh = *static_cast<UserData*>(pContext->m_pUserData);
            VertexStride& v = mesh.vertices[GetVertexIndex(pContext, iFace, iVert)];
            fvNormOut[0] = v.normal.x;
            fvNormOut[1] = v.normal.y;
            fvNormOut[2] = v.normal.z;
        }

        void GetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
        {
            UserData&   mesh = *static_cast<UserData*>(pContext->m_pUserData);
            VertexStride& v = mesh.vertices[GetVertexIndex(pContext, iFace, iVert)];
            fvTexcOut[0] = v.uv.x;
            fvTexcOut[1] = v.uv.y;
        }

        void SetTangentSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
        {
            UserData&   mesh = *static_cast<UserData*>(pContext->m_pUserData);
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


    AABB MeshUtils::CalculateMeshAABB(const Array<VertexStride>& vertices)
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

    void MeshUtils::CalcTangents(Array<VertexStride>& vertices, const Array<u32>& indices, bool useMikktspace)
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
}
