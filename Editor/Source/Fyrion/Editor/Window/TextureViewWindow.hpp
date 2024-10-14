#pragma once
#include "Fyrion/Editor/EditorTypes.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"


namespace Fyrion
{
    class FY_API TextureViewWindow : public EditorWindow
    {
    public:
        FY_BASE_TYPES(EditorWindow);


        void Init(u32 id, VoidPtr userData) override;
        void Draw(u32 id, bool& open) override;

        static void Open(Texture texture);
    private:
        Texture texture = {};
    };
}
