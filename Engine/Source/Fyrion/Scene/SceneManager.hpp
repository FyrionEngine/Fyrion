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
        SceneGlobals(StringView name);
        SceneObject* GetRootObject();

        usize GetObjectCount() const { return m_count; }

        friend class SceneManager;
        friend class SceneObject;
        friend class Component;

    private:
        SceneObject                m_rootObject;
        Array<Component*>          m_updatables{};
        std::queue<SceneObject*>   m_objectsToDestroy{};
        std::queue<Component*>     m_componentsToStart{};
        usize                      m_count{};
        bool                       m_updating{};

        std::queue<Pair<Component*, TypeHandler*>> m_enqueedToDestroy{};

        void SceneObjectAdded(SceneObject* sceneObject);
        void SceneObjectRemoved(SceneObject* sceneObject);

        void AddUpdatableComponent(Component* component);
        void RemoveUpdatableComponent(Component* component);
        void EnqueueDestroy(SceneObject* sceneObject);
        void EnqueueDestroy(Component* component, TypeHandler* typeHandler);
        void EnqueueStart(Component* component);
        void DoUpdate(f64 deltaTime);
    };

    class FY_API SceneManager
    {
    public:
        static SceneObject* CreateScene(const StringView& sceneName);
        static SceneObject* GetCurrentScene();
        static void         SetCurrenScene(SceneObject* newCurrentScene);

        static void RegisterType(NativeTypeHandler<SceneManager>& type);

        friend class SceneObject;

    private:
        static void ExecuteUpdate(f64 deltaTime);
    };
}
