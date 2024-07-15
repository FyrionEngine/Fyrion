#include "GraphicsAssets.hpp"

#include "Graphics.hpp"
#include "ShaderManager.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"
#include "cgltf.h"
#include "Fyrion/Scene/Assets/SceneObjectAsset.hpp"


namespace Fyrion
{
    class Logger;

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::GraphicsAssets");
    }

    Span<StringView> ShaderIO::GetImportExtensions()
    {
        return {extension, 2};
    }

    Asset* ShaderIO::CreateAsset()
    {
        return AssetDatabase::Create<ShaderAsset>();
    }

    void ShaderIO::ImportAsset(StringView path, Asset* asset)
    {
        ShaderAsset* shaderAsset = asset->Cast<ShaderAsset>();
        shaderAsset->SetShaderSource(FileSystem::ReadFileAsString(path));

        if (Path::Extension(path) == ".raster")
        {
            shaderAsset->SetShaderType(ShaderAssetType::Graphics);
        }
        else if (Path::Extension(path) == ".comp")
        {
            shaderAsset->SetShaderType(ShaderAssetType::Compute);
        }
        shaderAsset->Compile();
    }

    StringView ShaderAsset::GetDisplayName() const
    {
        if (shaderType == ShaderAssetType::Graphics)
        {
            return "Graphics Shader";
        }

        if (shaderType == ShaderAssetType::Compute)
        {
            return "Compute Shader";
        }

        if (shaderType == ShaderAssetType::Raytrace)
        {
            return "Raytrace Shader";
        }
        return "Shader";
    }

    Span<ShaderStageInfo> ShaderAsset::GetStages() const
    {
        return stages;
    }

    Span<u8> ShaderAsset::GetBytes() const
    {
        return bytes;
    }

    void ShaderAsset::Compile()
    {
        RenderApiType renderApi = Graphics::GetRenderApi();
        StringView    source = GetShaderSource();

        if (shaderType == ShaderAssetType::Graphics)
        {
            if (!ShaderManager::CompileShader(ShaderCreation{.source = source, .entryPoint = "MainVS", .shaderStage = ShaderStage::Vertex, .renderApi = renderApi}, bytes))
            {
                return;
            }

            stages.EmplaceBack(ShaderStageInfo{
                .stage = ShaderStage::Vertex,
                .entryPoint = "MainVS",
                .offset = 0,
                .size = (u32)bytes.Size()
            });

            u32 pixelOffset = (u32)bytes.Size();

            if (!ShaderManager::CompileShader(ShaderCreation{.source = source, .entryPoint = "MainPS", .shaderStage = ShaderStage::Pixel, .renderApi = renderApi}, bytes))
            {
                return;
            }

            stages.EmplaceBack(ShaderStageInfo{
                .stage = ShaderStage::Pixel,
                .entryPoint = "MainPS",
                .offset = pixelOffset,
                .size = (u32)bytes.Size() - pixelOffset
            });
        }
        else if (shaderType == ShaderAssetType::Compute)
        {
            if (ShaderManager::CompileShader(ShaderCreation{
                                                 .source = source,
                                                 .entryPoint = "MainCS",
                                                 .shaderStage = ShaderStage::Compute,
                                                 .renderApi = renderApi
                                             }, bytes))
            {
                stages.EmplaceBack(ShaderStageInfo{
                    .stage = ShaderStage::Compute,
                    .entryPoint = "MainCS",
                    .size = (u32)bytes.Size()
                });
            }
        }

        shaderInfo = ShaderManager::ExtractShaderInfo(bytes, stages, renderApi);
        logger.Debug("Shader {} compiled sucessfully", GetPath());
    }

    void ShaderAsset::RegisterType(NativeTypeHandler<ShaderAsset>& type)
    {
        type.Constructor<ShaderAsset>();
        type.Field<&ShaderAsset::shaderType>("shaderType");
    }

    Span<StringView> TextureIO::GetImportExtensions()
    {
        return {extension, 5};
    }

    Asset* TextureIO::CreateAsset()
    {
        return AssetDatabase::Create<TextureAsset>(UUID::RandomUUID());
    }

    void TextureIO::ImportAsset(StringView path, Asset* asset)
    {
        TextureAsset* textureAsset = asset->Cast<TextureAsset>();
        textureAsset->SetImage(path);
    }

    TextureAsset::~TextureAsset()
    {
        if (texture)
        {
            Graphics::DestroyTexture(texture);
        }
    }

    StringView TextureAsset::GetDisplayName() const
    {
        return "Texture";
    }

    void TextureAsset::SetImage(StringView path)
    {
        Image    image(path);
        Span<u8> imgData = image.GetData();

        SaveBlob(data, imgData.Data(), imgData.Size());

        width = image.GetWidth();
        height = image.GetHeight();
        channels = image.GetChannels();
    }

    Texture TextureAsset::GetTexture()
    {
        if (!texture)
        {
            texture = Graphics::CreateTexture(TextureCreation{
                .extent = {width, height, 1},
            });

            usize size = GetBlobSize(data);
            Array<u8> textureData(size);
            LoadBlob(data, textureData.Data(), textureData.Size());

            Graphics::UpdateTextureData(TextureDataInfo{
                .texture = texture,
                .data = textureData.Data(),
                .size = size,
                .extent = {width, height, 1}
            });
        }

        return texture;
    }

    void TextureAsset::RegisterType(NativeTypeHandler<TextureAsset>& type)
    {
        type.Field<&TextureAsset::width>("width");
        type.Field<&TextureAsset::height>("height");
        type.Field<&TextureAsset::channels>("channels");
        type.Field<&TextureAsset::data>("data");
    }

    void MaterialAsset::RegisterType(NativeTypeHandler<MaterialAsset>& type)
    {
        type.Field<&MaterialAsset::baseColor>("baseColor");
        type.Field<&MaterialAsset::baseColorTexture>("baseColorTexture");
        type.Field<&MaterialAsset::normalTexture>("normalTexture");
        type.Field<&MaterialAsset::normalMultiplier>("normalMultiplier");
        type.Field<&MaterialAsset::metallic>("metallic");
        type.Field<&MaterialAsset::metallicTexture>("metallicTexture");
        type.Field<&MaterialAsset::roughness>("roughness");
        type.Field<&MaterialAsset::roughnessTexture>("roughnessTexture");
        type.Field<&MaterialAsset::metallicRoughnessTexture>("metallicRoughnessTexture");
        type.Field<&MaterialAsset::aoTexture>("aoTexture");
        type.Field<&MaterialAsset::emissiveTexture>("emissiveTexture");
        type.Field<&MaterialAsset::emissiveFactor>("emissiveFactor");
        type.Field<&MaterialAsset::alphaCutoff>("alphaCutoff");
        type.Field<&MaterialAsset::alphaMode>("alphaMode");
        type.Field<&MaterialAsset::uvScale>("uvScale");
    }

    void MeshAsset::RegisterType(NativeTypeHandler<MeshAsset>& type)
    {
       type.Field<&MeshAsset::boundingBox>("boundingBox");
       type.Field<&MeshAsset::indicesCount>("indicesCount");
       type.Field<&MeshAsset::verticesCount>("verticesCount");
       type.Field<&MeshAsset::materials>("materials");
       type.Field<&MeshAsset::primitives>("primitives");
       type.Field<&MeshAsset::vertices>("vertices");
       type.Field<&MeshAsset::indices>("indices");
    }

    void DCCAsset::RegisterType(NativeTypeHandler<DCCAsset>& type)
    {
        type.Field<&DCCAsset::scaleFactor>("scaleFactor");
        type.Field<&DCCAsset::mergeMaterials>("mergeMaterials");
        type.Field<&DCCAsset::mergeMeshes>("mergeMeshes");
        type.Field<&DCCAsset::textures>("textures");
        type.Field<&DCCAsset::meshes>("meshes");
        type.Field<&DCCAsset::materials>("materials");
        type.Field<&DCCAsset::sceneObject>("sceneObject");
    }

    void DCCAsset::AddMesh(MeshAsset* mesh)
    {
        meshes.EmplaceBack(mesh);
    }

    Span<StringView> GLTFIO::GetImportExtensions()
    {
        return {extension, 2};
    }

    Asset* GLTFIO::CreateAsset()
    {
        return AssetDatabase::Create<DCCAsset>(UUID::RandomUUID());
    }

    MeshAsset* LoadGltfMesh(cgltf_data* data, cgltf_mesh& gltfMesh, u32 index)
    {
        MeshAsset* meshAsset = AssetDatabase::Create<MeshAsset>();
        meshAsset->SetUUID(UUID::RandomUUID());
        return meshAsset;
    }

    void GLTFIO::ImportAsset(StringView path, Asset* asset)
    {
        DCCAsset* dccAsset = static_cast<DCCAsset*>(asset);

        cgltf_options options = {};
        cgltf_data* data = nullptr;
        cgltf_result result = cgltf_parse_file(&options, path.CStr(), &data);
        if (result != cgltf_result_success)
        {
            logger.Error("GLTF Error on import file {} ", path);
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
                logger.Error("GLTF buffer file not found {}", path.CStr());
                return;
            }
            //TODO add bufferFile as file dependency to Asset*
        }

        if (cgltf_load_buffers(&options, data, path.CStr()) != cgltf_result_success)
        {
            logger.Error("GLTF Failed to load buffers {}", path.CStr());
            cgltf_free(data);
            return;
        }

        if (cgltf_validate(data) != cgltf_result_success)
        {
            logger.Error("GLTF Failed validation for {}", path.CStr());
            cgltf_free(data);
            return;
        }

        for (u32 m = 0; m < data->meshes_count; ++m)
        {
            dccAsset->AddMesh(LoadGltfMesh(data, data->meshes[m], m));
        }

        cgltf_free(data);
    }
}
