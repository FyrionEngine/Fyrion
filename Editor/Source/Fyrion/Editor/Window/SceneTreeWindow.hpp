#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    class FY_API SceneTreeWindow : public EditorWindow
    {
    public:
        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<SceneTreeWindow>& type);
    private:
    };
}