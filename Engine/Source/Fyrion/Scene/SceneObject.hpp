#pragma once

#include "Component.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/UUID.hpp"

namespace Fyrion {
    class RenderGraph;
}

namespace Fyrion
{
    class SceneObjectAsset;
    class TypeHandler;

    struct SceneGlobals
    {

    };

    class FY_API SceneObject final
    {
    public:
        FY_NO_COPY_CONSTRUCTOR(SceneObject);

        SceneObject(SceneGlobals* globals);
        SceneObject(SceneObjectAsset* asset);
        ~SceneObject();

        StringView         GetName() const;
        void               SetName(const StringView& p_name);
        SceneObject*       GetParent() const;
        Span<SceneObject*> GetChildren() const;
        void               SetUUID(UUID p_uuid);
        UUID               GetUUID() const;
        SceneObjectAsset*  GetPrototype() const;
        void               AddChild(SceneObject* sceneObject);
        void               AddChildAt(SceneObject* sceneObject, usize pos);
        void               RemoveChild(SceneObject* sceneObject);
        void               RemoveChildAt(usize pos);
        Component&         AddComponent(TypeID typeId);
        Component&         AddComponent(TypeHandler* typeHandler);
        void               RemoveComponent(Component* component);
        Span<Component*>   GetComponents() const;
        Component*         GetComponent(TypeID typeId) const;
        void               Destroy();
        ArchiveObject      Serialize(ArchiveWriter& writer) const;
        void               Deserialize(ArchiveReader& reader, ArchiveObject object);
        bool               IsAlive() const;
        void               SetAlive(bool p_alive);
        void               Notify(i64 type, VoidPtr userData);


        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        T& AddComponent()
        {
            return static_cast<T&>(AddComponent(GetTypeID<T>()));
        }

        template <typename T, Traits::EnableIf<Traits::IsBaseOf<Component, T>>* = nullptr>
        T* GetComponent()
        {
            return static_cast<T*>(GetComponent(GetTypeID<T>()));
        }

        static void RegisterType(NativeTypeHandler<SceneObject>& type);

        SceneGlobals*       globals{};
    private:
        bool                root = false;
        SceneObjectAsset*   asset{};
        String              name;
        UUID                uuid;
        Array<Component*>   components{};
        Array<SceneObject*> children{};
        SceneObject*        parent = nullptr;
        bool                alive = true;
        bool notificationDisabled = false;
    };

    template <>
    struct ArchiveType<SceneObject>
    {
        constexpr static bool hasArchiveImpl = true;

        static void Write(ArchiveWriter& writer, ArchiveObject object, StringView name, const SceneObject* value)
        {
            writer.WriteValue(object, name, value->Serialize(writer));
        }
        static void Read(ArchiveReader& reader, ArchiveObject object, StringView name, SceneObject* value)
        {
            value->Deserialize(reader, reader.ReadObject(object, name));
        }

        static void Add(ArchiveWriter& writer, ArchiveObject array, const SceneObject* value)
        {
            FY_ASSERT(false, "not implemented");
        }

        static void Get(ArchiveReader& reader, ArchiveObject item, SceneObject* value)
        {
            FY_ASSERT(false, "not implemented");
        }
    };
}
