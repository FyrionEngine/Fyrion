#pragma once
#include "Fyrion/Core/String.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion
{
    class SceneManager;

    class FY_API SceneObject
    {
    public:
        SceneObject(SceneObject* parent, StringView name);
        SceneObject(const SceneObject& other) = delete;
        SceneObject& operator=(const SceneObject& other) = delete;

        SceneObject* GetScene();

        friend class SceneManager;
    private:
        SceneObject* m_parent;
        String       m_name;

    };
}
