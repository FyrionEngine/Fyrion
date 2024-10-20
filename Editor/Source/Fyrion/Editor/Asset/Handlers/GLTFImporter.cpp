#include <cgltf.h>

#include "MeshAssetHandler.hpp"
#include "TextureAssetHandler.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/StringUtils.hpp"
#include "Fyrion/Editor/Asset/AssetEditor.hpp"
#include "Fyrion/Editor/Asset/AssetTypes.hpp"
#include "Fyrion/Graphics/Assets/MeshAsset.hpp"
#include "Fyrion/Graphics/Assets/TextureAsset.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{

    using ImportedTextureMap = HashMap<usize, TextureAsset*>;
    using ImportedMaterialMap = HashMap<usize, MaterialAsset*>;
    using ImportedMeshMap = HashMap<usize, MeshAsset*>;

    class GLTFImporter : public AssetImporter
    {
    public:

        FY_BASE_TYPES(AssetImporter);

        inline static Logger& logger = Logger::GetLogger("Fyrion::GLTFImporter");


        Array<String> ImportExtensions() override
        {
            return {".gltf", ".glb"};
        }

        MeshAsset* LoadGltfMesh(AssetFile* parent, ImportedMaterialMap& materialMap, cgltf_data* data, cgltf_mesh& gltfMesh, u32 index)
        {
            String name = gltfMesh.name != nullptr ? gltfMesh.name : String{"Mesh_"}.Append(ToString(index));

            Array<VertexStride>    vertices;
            Array<u32>             indices;
            Array<MeshPrimitive>   primitives;
            Array<MaterialAsset*>  materials;
            Array<cgltf_material*> gltfMaterials;
            bool                   missingNormals = false;
            bool                   missingTangents = false;

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
                            .uv = !texCoords.Empty() ? Math::MakeVec2(&texCoords[v * 2]) : Vec2{-1, 1},
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

                if (gltfPrimitive.material != nullptr)
                {
                    bool found = false;
                    for (auto m = 0; m < gltfMaterials.Size(); ++m)
                    {
                        if (gltfMaterials[m] == gltfPrimitive.material)
                        {
                            materialIndex = m;
                            found = true;
                            break;
                        }
                    }

                    if (!found)
                    {
                        if (auto it = materialMap.Find(reinterpret_cast<usize>(gltfPrimitive.material)))
                        {
                            materialIndex = materials.Size();

                            materials.EmplaceBack(it->second);
                            gltfMaterials.EmplaceBack(gltfPrimitive.material);
                        }
                    }
                }

                primitives.EmplaceBack(MeshPrimitive{
                    .firstIndex = firstIndex,
                    .indexCount = indexCount,
                    .materialIndex = materialIndex
                });
            }

            AssetFile* assetFile = AssetEditor::CreateAsset(parent, GetTypeID<MeshAsset>(), name);
            MeshAsset* meshAsset = Assets::Load<MeshAsset>(assetFile->uuid);

            MeshImportData importData = {
                .assetFile = assetFile,
                .meshAsset = meshAsset,
                .vertices = vertices,
                .indices = indices,
                .primitives = primitives,
                .materials = materials,
                .missingNormals = missingNormals,
                .missingTangents = missingTangents
            };

            MeshImporter::ImportMesh(importData);

            return meshAsset;
        }

        bool ImportAsset(AssetFile* parent, StringView path) override
        {
            cgltf_options options = {};
            cgltf_data*   data = nullptr;
            cgltf_result  result = cgltf_parse_file(&options, path.CStr(), &data);
            if (result != cgltf_result_success)
            {
                logger.Error("Error on import file {} ", path);
                return false;
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

                String bufferPath = Path::Join(Path::Parent(path), uri);

                if (!FileSystem::GetFileStatus(bufferPath).exists)
                {
                    logger.Error("buffer file not found {} ", path);
                    return false;
                }
            }

            if (cgltf_load_buffers(&options, data, path.CStr()) != cgltf_result_success)
            {
                logger.Error("Failed to load buffers {}", path.CStr());
                cgltf_free(data);
                return false;
            }

            if (cgltf_validate(data) != cgltf_result_success)
            {
                logger.Error("Failed validation for {}", path.CStr());
                cgltf_free(data);
                return false;
            }

            ImportedTextureMap  textureMap;
            ImportedMaterialMap materialMap;
            ImportedMeshMap     meshMap;

            for (i32 t = 0; t < data->textures_count; ++t)
            {
                const cgltf_texture& texture = data->textures[t];

                if (texture.image->buffer_view != nullptr)
                {
                    String textureName = texture.name != nullptr ? texture.name : String{"Texture_"}.Append(ToString(t));

                    AssetFile* assetFile = AssetEditor::CreateAsset(parent, GetTypeID<TextureAsset>(), textureName);
                    TextureAsset* textureAsset = Assets::Load<TextureAsset>(assetFile->uuid);

                    Span<const u8> imageBuffer{
                        static_cast<const u8*>(texture.image->buffer_view->buffer->data) + texture.image->buffer_view->offset,
                        static_cast<const u8*>(texture.image->buffer_view->buffer->data) + texture.image->buffer_view->offset + texture.image->buffer_view->size
                    };

                    TextureImporter::ImportTexture(assetFile, textureAsset, imageBuffer);

                    textureMap.Insert(reinterpret_cast<usize>(&texture), textureAsset);
                }
            }

            for (u32 m = 0; m < data->meshes_count; ++m)
            {
                MeshAsset* meshAsset = LoadGltfMesh(parent, materialMap, data, data->meshes[m], m);
                meshMap.Insert(reinterpret_cast<usize>(&data->meshes[m]), meshAsset);
            }

            cgltf_free(data);
            return true;
        }
    };

    void RegisterGLTFImporter()
    {
        Registry::Type<GLTFImporter>();
    }
}
