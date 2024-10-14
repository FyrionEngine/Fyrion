#include "TextureViewWindow.hpp"

#include "Fyrion/Editor/Editor.hpp"
#include "Fyrion/ImGui/ImGui.hpp"


namespace Fyrion
{

    void TextureViewWindow::Init(u32 id, VoidPtr userData)
    {
        texture = *static_cast<Texture*>(userData);
    }


    void TextureViewWindow::Draw(u32 id, bool& open)
    {
        ImGui::Begin(id, "Texture View", &open, ImGuiWindowFlags_NoScrollbar);
        ImGui::TextureItem(texture, ImGui::GetWindowSize());
        ImGui::End();
    }



    void TextureViewWindow::Open(Texture texture)
    {
        Editor::OpenWindow<TextureViewWindow>(&texture);
    }
}
