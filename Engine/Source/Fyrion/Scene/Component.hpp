#pragma once

#include "SceneTypes.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/UUID.hpp"


namespace Fyrion
{
    class SceneObject;

    class FY_API Component
    {
    public:
        TypeHandler* typeHandler = nullptr;
        SceneObject* object = nullptr;
        virtual      ~Component() = default;

        virtual void OnStart() {}
        virtual void OnChange() {}
        virtual void OnDestroy() {}
        virtual void OnNotify(const NotificationEvent& notificationEvent) {}

        void        SetUUID(const UUID& uuid);
        const UUID& GetUUID() const;

        void SetPrototype(const UUID& prototype);
        UUID GetPrototype() const;

        static void RegisterType(NativeTypeHandler<Component>& type);

    private:
        UUID uuid{};
        UUID prototype{};
    };
}
