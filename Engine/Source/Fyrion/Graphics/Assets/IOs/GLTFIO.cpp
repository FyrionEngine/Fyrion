#include <cgltf.h>

#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Graphics/Assets/DCCAsset.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    struct FY_API GLTFIO : AssetIO
    {
        FY_BASE_TYPES(AssetIO);

        Logger& logger = Logger::GetLogger("Fyrion::GLTFIO");

        StringView extension[2] = {".gltf", ".glb"};

        Span<StringView> GetImportExtensions() override
        {
            return {extension, 2};
        }

        Asset* CreateAsset() override
        {
            return AssetDatabase::Create<DCCAsset>(UUID::RandomUUID());
        }

        void LoadGltfMesh(DCCAsset* dccAsset, cgltf_data* data, cgltf_mesh& gltfMesh, u32 index)
        {
            String name = gltfMesh.name != nullptr ? gltfMesh.name : String{"Mesh_"}.Append(index);

            Array<VertexStride>   vertices{};
            Array<u32>            indices{};
            Array<MeshPrimitive>  primitives{};
            Array<MaterialAsset*> materials{};
            bool                  missingNormals{false};
            bool                  missingTangents{false};

            MeshAsset* meshAsset =  AssetDatabase::Create<MeshAsset>();
            meshAsset->SetName(name);
            meshAsset->SetUUID(UUID::RandomUUID());
            meshAsset->SetOwner(dccAsset);

            for (u32 p = 0; p < gltfMesh.primitives_count; ++p)
            {
                cgltf_primitive& gltfPrimitive = gltfMesh.primitives[p];
                if (gltfPrimitive.type != cgltf_primitive_type_triangles) continue; // TODO primitive type.

                u32 firstIndex = static_cast<uint32_t>(indices.Size());
                u32 vertexStart = static_cast<uint32_t>(vertices.Size());
                u32 indexCount = 0;

                Span<f32> positions{};
                Span<f32> normals{};
                Span<f32> colors{};
                Span<f32> texCoords{};
                Span<f32> tangents{};

                for (u32 a = 0; a < gltfPrimitive.attributes_count; ++a)
                {
                    cgltf_attribute& attribute = gltfPrimitive.attributes[a];
                    cgltf_accessor*  accessor = attribute.data;

                    if (!accessor->buffer_view->buffer->data) continue;

                    switch (attribute.type)
                    {
                        case cgltf_attribute_type_position:
                        {
                            positions = {
                                (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32),
                                (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32) + accessor->count
                            };
                            break;
                        }
                        case cgltf_attribute_type_texcoord:
                        {
                            texCoords = {
                                (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32),
                                (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32) + accessor->count
                            };
                            break;
                        }
                        case cgltf_attribute_type_normal:
                        {
                            normals = {
                                (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32),
                                (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32) + accessor->count
                            };
                            break;
                        }
                        case cgltf_attribute_type_color:
                        {
                            if (accessor->component_type == cgltf_component_type_r_8u)
                            {
                                colors = {
                                    (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32),
                                    (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32) + accessor->count
                                };
                            }
                            break;
                        }
                        case cgltf_attribute_type_tangent:
                        {
                            tangents = {
                                (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32),
                                (f32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(f32) + accessor->offset / sizeof(f32) + accessor->count
                            };
                            break;
                        }
                        case cgltf_attribute_type_joints:
                        case cgltf_attribute_type_weights:
                        case cgltf_attribute_type_custom:
                        case cgltf_attribute_type_max_enum:
                        case cgltf_attribute_type_invalid:
                            //ignored
                            break;
                    }
                }

                if (normals.Empty())
                {
                    missingNormals = true;
                }

                if (tangents.Empty())
                {
                    missingTangents = true;
                }

                if (positions.Data() != nullptr && !positions.Empty())
                {
                    for (int v = 0; v < positions.Size(); ++v)
                    {
                        vertices.EmplaceBack(VertexStride{
                            .position = Math::MakeVec3(&positions[v * 3]),
                            .normal = !normals.Empty() ? Math::MakeVec3(&normals[v * 3]) : Vec3{},
                            .color = !colors.Empty() ? Math::MakeVec3(&colors[v * 3]) : Vec3{1, 1, 1},
                            .uv = !texCoords.Empty() ? Math::MakeVec2(&texCoords[v * 2]) : Vec2{},
                            .tangent = !tangents.Empty() ? Math::MakeVec4(&tangents[v * 4]) : Vec4{},
                        });
                    }
                }

                if (gltfPrimitive.indices != nullptr)
                {
                    cgltf_accessor* accessor = gltfPrimitive.indices;

                    if (!accessor->buffer_view->buffer->data) continue;

                    indices.Reserve(indices.Size() + accessor->count);

                    indexCount = accessor->count;

                    if (accessor->component_type == cgltf_component_type_r_32u)
                    {
                        u32* buffer = (u32*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(u32) + accessor->offset / sizeof(u32);
                        for (u32 index = 0; index < accessor->count; ++index)
                        {
                            indices.EmplaceBack(buffer[index] + vertexStart);
                        }
                    }
                    else if (accessor->component_type == cgltf_component_type_r_16u)
                    {
                        u16* buffer = (u16*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(u16) + accessor->offset / sizeof(u16);
                        for (u32 index = 0; index < accessor->count; ++index)
                        {
                            indices.EmplaceBack(buffer[index] + vertexStart);
                        }
                    }
                    else if (accessor->component_type == cgltf_component_type_r_8u)
                    {
                        u8* buffer = (u8*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset / sizeof(u8) + accessor->offset / sizeof(u8);
                        for (u32 index = 0; index < accessor->count; ++index)
                        {
                            indices.EmplaceBack(buffer[index] + vertexStart);
                        }
                    }
                }

                u32 materialIndex = 0;
                for (unsigned int m = 0; m < data->materials_count; m++)
                {
                    if (&data->materials[m] == gltfPrimitive.material)
                    {
                        materialIndex = m;
                        break;
                    }
                }

                u32 materialMeshIndex = U32_MAX;

                primitives.EmplaceBack(MeshPrimitive{
                    .firstIndex = firstIndex,
                    .indexCount = indexCount,
                    .materialIndex = materialMeshIndex != U32_MAX ? materialMeshIndex : 0
                });
            }

            meshAsset->SetData(vertices, indices, primitives, materials, missingNormals, missingTangents);
        }

        void ImportAsset(StringView path, Asset* asset) override
        {
            DCCAsset* dccAsset = static_cast<DCCAsset*>(asset);

            cgltf_options options = {};
            cgltf_data*   data = nullptr;
            cgltf_result  result = cgltf_parse_file(&options, path.CStr(), &data);
            if (result != cgltf_result_success)
            {
                logger.Error("Error on import file {} ", path);
                return;
            }

            for (cgltf_size i = 0; i < data->buffers_count; ++i)
            {
                if (data->buffers[i].data)
                {
                    continue;
                }

                const char* uri = data->buffers[i].uri;

                if (uri == nullptr)
                {
                    continue;
                }

                if (strncmp(uri, "data:", 5) == 0)
                {
                    continue;
                }

                String bufferFile = Path::Join(Path::Parent(path), uri);
                if (!FileSystem::GetFileStatus(bufferFile).exists)
                {
                    logger.Error("buffer file not found {}", path.CStr());
                    return;
                }
                //TODO add bufferFile as file dependency to Asset*
            }

            if (cgltf_load_buffers(&options, data, path.CStr()) != cgltf_result_success)
            {
                logger.Error("Failed to load buffers {}", path.CStr());
                cgltf_free(data);
                return;
            }

            if (cgltf_validate(data) != cgltf_result_success)
            {
                logger.Error("Failed validation for {}", path.CStr());
                cgltf_free(data);
                return;
            }

            for (u32 m = 0; m < data->meshes_count; ++m)
            {
                LoadGltfMesh(dccAsset, data, data->meshes[m], m);
            }

            cgltf_free(data);
        }
    };

    void RegisterGLTFIO()
    {
        Registry::Type<GLTFIO>();
    }
}
