#pragma once

#include "Fyrion/Core/Registry.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"

namespace Fyrion
{
    class WorldViewWindow : public EditorWindow
    {
    public:
        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<WorldViewWindow>& type);
    private:
        static void OpenWorldView(VoidPtr userData);
    };
}
