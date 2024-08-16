#include "Asset.hpp"

#include "AssetHandler.hpp"
#include "AssetSerialization.hpp"
#include "Fyrion/Core/Logger.hpp"

namespace Fyrion
{
    AssetHandler* Asset::GetHandler() const
    {
        return handler;
    }

    void Asset::SetHandler(AssetHandler* handler)
    {
        this->handler = handler;
    }

    void Asset::SetModified()
    {
        handler->SetModified();
    }

    ArchiveObject Asset::Serialize(ArchiveWriter& writer) const
    {
        return Serialization::Serialize(handler->GetType(), writer, this);
    }

    void Asset::Deserialize(ArchiveReader& reader, ArchiveObject object)
    {
        Serialization::Deserialize(handler->GetType(), reader, object, this);
    }

    void Asset::SaveBuffer(AssetBuffer& buffer, ConstPtr data, usize dataSize)
    {
        if (AssetBufferManager* bufferManager = handler->GetBufferManager())
        {
            bufferManager->SaveBuffer(buffer, data, dataSize);
        }
    }

    Array<u8> Asset::LoadBuffer(AssetBuffer buffer) const
    {
        if (AssetBufferManager* bufferManager = handler->GetBufferManager())
        {
            return bufferManager->LoadBuffer(buffer);
        }
        return {};
    }

    bool Asset::HasBuffer(AssetBuffer buffer) const
    {
        if (AssetBufferManager* bufferManager = handler->GetBufferManager())
        {
            return bufferManager->HasBuffer(buffer);
        }
        return false;
    }

    Asset* Asset::GetParent() const
    {
        if (handler->GetParent() != nullptr)
        {
            return handler->GetParent()->LoadInstance();
        }
        return nullptr;
    }

    void Asset::RegisterType(NativeTypeHandler<Asset>& type)
    {

    }
}
