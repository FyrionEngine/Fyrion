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

        MeshAsset* LoadGltfMesh(cgltf_data* data, cgltf_mesh& gltfMesh, u32 index)
        {
            MeshAsset* meshAsset = AssetDatabase::Create<MeshAsset>();
            meshAsset->SetUUID(UUID::RandomUUID());
            return meshAsset;
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
                dccAsset->AddMesh(LoadGltfMesh(data, data->meshes[m], m));
            }

            cgltf_free(data);
        }
    };

    void RegisterGLTFIO()
    {
        Registry::Type<GLTFIO>();
    }
}
