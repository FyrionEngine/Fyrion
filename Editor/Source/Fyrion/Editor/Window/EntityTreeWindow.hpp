#pragma once

#include "Fyrion/Common.hpp"
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    class FY_API EntityTreeWindow : public EditorWindow
    {
    public:
        void Draw(u32 id, bool& open) override;

        static void RegisterType(NativeTypeHandler<EntityTreeWindow>& type);
    private:
        static void OpenEntityTree(VoidPtr userData);
    };
}