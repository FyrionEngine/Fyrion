

#include "Fyrion/Editor/Asset/AssetTypes.hpp"

namespace Fyrion
{
    struct JsonAssetHandler : AssetHandler
    {
        FY_BASE_TYPES(AssetHandler);

        StringView Extension() override
        {
            return ".asset";
        }

        void Save(AssetFile* assetFile) override
        {
            //ArchiveWriter archiveWriter;

           // Serialization::Serialize()
        }
    };
}
