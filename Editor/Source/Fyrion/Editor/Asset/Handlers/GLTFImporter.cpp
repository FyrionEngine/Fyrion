#include <cgltf.h>

#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Editor/Asset/AssetTypes.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    class GLTFImporter : AssetImporter
    {
    public:
        inline static Logger& logger = Logger::GetLogger("Fyrion::GLTFImporter");


        Array<String> ImportExtensions() override
        {
            return {".gltf", ".glb"};
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


            return true;
        }
    };
}
