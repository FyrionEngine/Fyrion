#include <cgltf.h>

#include "Fyrion/Asset/AssetHandler.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"
#include "Fyrion/Core/Image.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Graphics/Assets/DCCAsset.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Fyrion/Scene/SceneManager.hpp"
#include "Fyrion/Scene/Components/MeshRender.hpp"
#include "Fyrion/Scene/Components/TransformComponent.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::GLTFIO");
    }

    struct FY_API GLTFIO : AssetIO
    {
        using ImportedTextureMap = HashMap<usize, TextureAsset*>;
        using ImportedMaterialMap = HashMap<usize, MaterialAsset*>;
        using ImportedMeshMap = HashMap<usize, MeshAsset*>;

        FY_BASE_TYPES(AssetIO);

        static void MoveAsset(AssetHandler* asset, StringView newName, AssetHandler* newParent)
        {
            for(const String& file: asset->GetRelatedFiles())
            {
                String oldPath = Path::Join(asset->GetParent()->GetAbsolutePath(), file);
                String newPath = Path::Join(newParent->GetAbsolutePath(), file);
                FileSystem::Rename(oldPath, newPath);
            }
        }

        GLTFIO()
        {
            getImportExtensions = GetImportExtensions;
            getAssetTypeId = GetAssetTypeID;
            importAsset = ImportAsset;
            renameAsset = MoveAsset;
        }

        static inline StringView extension[2] = {".gltf", ".glb"};

        static Span<StringView> GetImportExtensions()
        {
            return {extension, 2};
        }

        static TypeID GetAssetTypeID(StringView path)
        {
            return GetTypeID<DCCAsset>();
        }

        static MeshAsset* LoadGltfMesh(DCCAsset* dccAsset, ImportedMaterialMap& materialMap, cgltf_data* data, cgltf_mesh& gltfMesh, u32 index)
        {
            String name = gltfMesh.name != nullptr ? gltfMesh.name : String{"Mesh_"}.Append(index);

            Array<VertexStride>    vertices;
            Array<u32>             indices;
            Array<MeshPrimitive>   primitives;
            Array<MaterialAsset*>  materials;
            Array<cgltf_material*> gltfMaterials;
            bool                   missingNormals = false;
            bool                   missingTangents = false;

            MeshAsset* meshAsset = dccAsset->FindMeshByName(name);
            if (meshAsset == nullptr)
            {
                meshAsset = AssetManager::Create<MeshAsset>(AssetCreation{
                    .name = name,
                    .parent = dccAsset->GetHandler()
                });
            }

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

            meshAsset->SetData(vertices, indices, primitives, materials, missingNormals, missingTangents);

            return meshAsset;
        }

        static void LoadGltfNode(const ImportedMeshMap& meshMap, SceneObject* parentObject, cgltf_node* node, u32& meshCount)
        {
            String nodeName = node->name != nullptr ? String{node->name}.Append("_").Append(meshCount++) : String("MeshNode_").Append(meshCount++);

            SceneObject* object = parentObject->FindChildByName(nodeName);
            if (object == nullptr)
            {
                object = SceneManager::CreateObject();
                object->SetName(nodeName);
                object->SetUUID(UUID::RandomUUID());
                parentObject->AddChild(object);
                TransformComponent& transformComponent = object->CreateComponent<TransformComponent>();
                MeshRender& meshRender = object->CreateComponent<MeshRender>();

                transformComponent.SetUUID(UUID::RandomUUID());
                meshRender.SetUUID(UUID::RandomUUID());
            }

            if (node->mesh)
            {
                if (auto it = meshMap.Find(reinterpret_cast<usize>(node->mesh)))
                {
                    MeshRender* meshRender = object->GetComponent<MeshRender>();
                    meshRender->SetMesh(it->second);
                }
            }

            //TODO : import camera and lights

            Vec3 position{0, 0, 0};
            Quat rotation{0, 0, 0, 1};
            Vec3 scale{1, 1, 1};

            if (node->has_translation)
            {
                position = Vec3{node->translation[0], node->translation[1], node->translation[2]};
            }

            if (node->has_rotation)
            {
                rotation = Quat{node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]};
            }

            if (node->has_scale)
            {
                scale = Vec3{node->scale[0], node->scale[1], node->scale[2]};
            }

            if (node->has_matrix)
            {
                Mat4 mat = MakeMat4(node->matrix);
                position = Math::GetTranslation(mat);
                scale = Math::GetScale(mat);
                rotation = Math::GetQuaternion(mat);
            }

            TransformComponent* transformComponent = object->GetComponent<TransformComponent>();
            transformComponent->SetTransform(position, rotation, scale);

            for (u32 c = 0; c < node->children_count; ++c)
            {
                LoadGltfNode(meshMap, object, node->children[c], meshCount);
            }
        }


        static TextureAsset* FindTexture(const ImportedTextureMap& textureMap, DCCAsset* dccAsset, cgltf_texture* texture)
        {
            if (texture->image->uri != nullptr)
            {
                String texturePath = String(dccAsset->GetHandler()->GetParent()->GetPath()).Append("/").Append(StringView{texture->image->uri});

                TextureAsset* textureAsset = AssetManager::LoadByPath<TextureAsset>(texturePath);
                if (textureAsset == nullptr)
                {
                    // textureAsset = AssetDatabase::Create<TextureAsset>();
                    // textureAsset->SetPath(texturePath);
                    logger.Error("texture {} not found", texture->image->uri);
                }

                return textureAsset;
            }

            if (auto it = textureMap.Find(reinterpret_cast<usize>(texture)))
            {
                return it->second;
            }

            return nullptr;
        }

        static bool ImportAsset(StringView path, Asset* asset)
        {
            DCCAsset* dccAsset = static_cast<DCCAsset*>(asset);

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
                    bufferPath = Path::Join(Path::Parent(path), asset->GetHandler()->GetName(), Path::Extension(uri));
                }

                if (!FileSystem::GetFileStatus(bufferPath).exists)
                {
                    logger.Error("buffer file not found {}", path.CStr());
                    cgltf_free(data);
                    return false;
                }

                String bufferName = Path::Name(bufferPath) + Path::Extension(bufferPath);
                if (data->buffers[i].uri)
                {
                    data->memory.free_func(data->memory.user_data, data->buffers[i].uri);
                    data->buffers[i].uri = (char*)data->memory.alloc_func(data->memory.user_data, bufferName.Size() + 1);
                    MemCopy(data->buffers[i].uri, bufferName.CStr(), bufferName.Size());
                    data->buffers[i].uri[bufferName.Size()] = 0;
                }
                asset->GetHandler()->AddRelatedFile(bufferName);
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
                    String        textureName = texture.name != nullptr ? texture.name : String{"Texture_"}.Append(t);
                    TextureAsset* textureAsset = dccAsset->FindTextureByName(textureName);
                    if (textureAsset == nullptr)
                    {
                        textureAsset = AssetManager::Create<TextureAsset>(AssetCreation{
                            .name = textureName,
                            .parent = dccAsset->GetHandler(),
                        });
                    }

                    Span<const u8> imageBuffer{
                        static_cast<const u8*>(texture.image->buffer_view->buffer->data) + texture.image->buffer_view->offset,
                        static_cast<const u8*>(texture.image->buffer_view->buffer->data) + texture.image->buffer_view->offset + texture.image->buffer_view->size
                    };

                    Image image{imageBuffer};
                    textureAsset->SetImage(image);

                    textureMap.Insert(reinterpret_cast<usize>(&texture), textureAsset);
                }
            }

            for (int m = 0; m < data->materials_count; ++m)
            {
                const cgltf_material& material = data->materials[m];
                String                materialName = material.name != nullptr ? material.name : String{"Material_"}.Append(m);

                MaterialAsset* materialAsset = dccAsset->FindMaterialByName(materialName);
                if (materialAsset == nullptr)
                {
                    materialAsset = AssetManager::Create<MaterialAsset>(AssetCreation{
                        .name = materialName,
                        .parent = dccAsset->GetHandler(),
                    });
                }

                if (material.has_pbr_metallic_roughness)
                {
                    materialAsset->SetBaseColor(Color::FromVec4Gamma(material.pbr_metallic_roughness.base_color_factor));
                    materialAsset->SetUvScale(Vec2{material.pbr_metallic_roughness.base_color_texture.scale, material.pbr_metallic_roughness.base_color_texture.scale});

                    if (material.pbr_metallic_roughness.base_color_texture.texture)
                    {
                        materialAsset->SetBaseColorTexture(FindTexture(textureMap, dccAsset, material.pbr_metallic_roughness.base_color_texture.texture));
                    }

                    if (material.pbr_metallic_roughness.metallic_roughness_texture.texture)
                    {
                        materialAsset->SetMetallicRoughnessTexture(FindTexture(textureMap, dccAsset, material.pbr_metallic_roughness.metallic_roughness_texture.texture));
                    }
                }
                else if (material.has_pbr_specular_glossiness)
                {
                    materialAsset->SetBaseColor(Color::FromVec4(material.pbr_specular_glossiness.diffuse_factor));
                }

                materialAsset->SetAlphaCutoff(material.alpha_cutoff);

                if (material.normal_texture.texture)
                {
                    materialAsset->SetNormalTexture(FindTexture(textureMap, dccAsset, material.normal_texture.texture));
                }

                if (material.occlusion_texture.texture)
                {
                    materialAsset->SetAoTexture(FindTexture(textureMap, dccAsset, material.occlusion_texture.texture));
                }

                if (material.emissive_texture.texture)
                {
                    materialAsset->SetEmissiveTexture(FindTexture(textureMap, dccAsset, material.emissive_texture.texture));
                }

                materialMap.Insert(reinterpret_cast<usize>(&material), materialAsset);
            }

            for (u32 m = 0; m < data->meshes_count; ++m)
            {
                MeshAsset* meshAsset = LoadGltfMesh(dccAsset, materialMap, data, data->meshes[m], m);
                meshMap.Insert(reinterpret_cast<usize>(&data->meshes[m]), meshAsset);
            }

            if (data->scenes_count > 0)
            {
                SceneObjectAsset* rootObjectAsset = dccAsset->GetSceneObjectAsset();
                if (rootObjectAsset == nullptr)
                {
                    rootObjectAsset = AssetManager::Create<SceneObjectAsset>(AssetCreation{
                        .name = dccAsset->GetHandler()->GetName(),
                        .parent = dccAsset->GetHandler()
                    });

                    TransformComponent& transformComponent = rootObjectAsset->GetObject()->CreateComponent<TransformComponent>();
                    transformComponent.SetUUID(UUID::RandomUUID());
                }

                SceneObject* rootObject = rootObjectAsset->GetObject();

                u32 meshCount = 0;
                for (u32 c = 0; c < data->scenes_count; ++c)
                {
                    cgltf_scene& scene = data->scenes[c];
                    for (u32 n = 0; n < scene.nodes_count; ++n)
                    {
                        LoadGltfNode(meshMap, rootObject, scene.nodes[n], meshCount);
                    }
                }
            }

            cgltf_free(data);
            return true;
        }
    };

    void RegisterGLTFIO()
    {
        Registry::Type<GLTFIO>();
    }
}
