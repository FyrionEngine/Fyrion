#include "Asset.hpp"

#include "AssetHandler.hpp"
#include "AssetSerialization.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/IO/FileSystem.hpp"
#include "Fyrion/IO/Path.hpp"

namespace Fyrion
{
    AssetHandler* Asset::GetInfo() const
    {
        return info;
    }

    void Asset::SetInfo(AssetHandler* info)
    {
        this->info = info;
    }

    void Asset::SetModified()
    {
        info->SetModified();
    }

    ArchiveObject Asset::Serialize(ArchiveWriter& writer) const
    {
        return Serialization::Serialize(info->GetType(), writer, this);
    }

    void Asset::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        Serialization::Deserialize(info->GetType(), reader, object, this);
    }

    void Asset::SaveBuffer(AssetBuffer& buffer, ConstPtr data, usize dataSize)
    {
        if (AssetBufferManager* bufferManager = info->GetBufferManager())
        {
            bufferManager->SaveBuffer(buffer, data, dataSize);
        }
    }

    Array<u8> Asset::LoadBuffer(AssetBuffer buffer) const
    {
        if (AssetBufferManager* bufferManager = info->GetBufferManager())
        {
            return bufferManager->LoadBuffer(buffer);
        }
        return {};
    }

    bool Asset::HasBuffer(AssetBuffer buffer) const
    {
        if (AssetBufferManager* bufferManager = info->GetBufferManager())
        {
            return bufferManager->HasBuffer(buffer);
        }
        return false;
    }

    Asset* Asset::GetParent() const
    {
        if (info->GetParent() != nullptr)
        {
            return info->GetParent()->LoadInstance();
        }
        return nullptr;
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {

    }
}
