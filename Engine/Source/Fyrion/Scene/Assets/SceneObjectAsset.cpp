#include "SceneObjectAsset.hpp"
#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Graphics/RenderGraph.hpp"
#include "Fyrion/Scene/SceneManager.hpp"


namespace Fyrion
{
    SceneObject* SceneObjectAsset::GetObject()
    {
        if (!object)
        {
            if (!LoadData())
            {
                object = SharedPtr(MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>(this));
            }
        }
        pendingDestroy = false;
        return object.Get();
    }

    void SceneObjectAsset::DestroySceneObject()
    {
        if (!IsModified())
        {
            object = {};
        }
        else
        {
            pendingDestroy = true;
        }
    }

    void SceneObjectAsset::DeserializeData(ArchiveReader& reader, ArchiveObject archiveObject)
    {
        if (!object)
        {
            object = SharedPtr(MemoryGlobals::GetDefaultAllocator().Alloc<SceneObject>(this));
            object->Deserialize(reader, archiveObject);
        }
    }

    ArchiveObject SceneObjectAsset::SerializeData(ArchiveWriter& writer) const
    {
        if (object)
        {
            return object->Serialize(writer);
        }
        return {};
    }

    bool SceneObjectAsset::LoadData()
    {
        pendingDestroy = false;
        return Asset::LoadData();
    }

    void SceneObjectAsset::SaveData()
    {
        Asset::SaveData();
        if (pendingDestroy)
        {
            pendingDestroy = false;
            object = {};
        }
    }

    void SceneObjectAsset::RegisterType(NativeTypeHandler<SceneObjectAsset>& type)
    {
    }
}
