#pragma once

#include <queue>

#include "SceneObject.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion
{
    class FY_API SceneGlobals
    {
    public:
        SceneGlobals(StringView name, RID asset);
        SceneObject* GetRootObject();

        friend class SceneManager;
        friend class SceneObject;
        friend class Component;

    private:
        HashMap<RID, SceneObject*> objectsByRID{};
        SceneObject                m_rootObject;
        std::queue<SceneObject*>   m_objectsToDestroy{};
        Array<Component*>          m_updatables{};

        void AddUpdatableComponent(Component* component);
        void RemoveUpdatableComponent(Component* component);
        void EnqueueDestroy(SceneObject* sceneObject);
        void DoUpdate(f64 deltaTime);
    };

    class FY_API SceneManager
    {
    public:
        static SceneObject* LoadScene(RID rid);
        static SceneObject* CreateScene(const StringView& sceneName);
        static SceneObject* GetCurrentScene();
        static void         SetCurrenScene(SceneObject* newCurrentScene);

        static void RegisterType(NativeTypeHandler<SceneManager>& type);

        friend class SceneObject;

    private:
        static void ExecuteUpdate(f64 deltaTime);
    };
}
