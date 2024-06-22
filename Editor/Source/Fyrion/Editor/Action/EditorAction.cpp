#include "EditorAction.hpp"

#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{

    void InitAssetEditorActions();

    EditorTransaction::~EditorTransaction()
    {
        for(const auto& action : actions)
        {
            action.first->Destroy(action.second);
        }
        actions.Clear();
        actions.ShrinkToFit();
    }

    EditorAction* EditorTransaction::CreateAction(TypeID typeId, VoidPtr* params, TypeID* paramTypes, usize paramNum)
    {
        if (TypeHandler* typeHandler = Registry::FindTypeById(typeId))
        {
            if (ConstructorHandler* constructor = typeHandler->FindConstructor(paramTypes, paramNum))
            {
                VoidPtr instance = constructor->NewInstance(MemoryGlobals::GetDefaultAllocator(), params);
                if (EditorAction* editorAction = typeHandler->Cast<EditorAction>(instance))
                {
                    actions.EmplaceBack(MakePair(typeHandler, editorAction));
                    return editorAction;
                }
                typeHandler->Destroy(instance);
            }
        }
        FY_ASSERT(false, "action cannot be created");
        return nullptr;
    }

    void EditorTransaction::Commit()
    {
        for(const auto& action : actions)
        {
            action.second->Commit();
        }
    }

    void EditorTransaction::Rollback()
    {
        for(const auto& action : actions)
        {
            action.second->Rollback();
        }
    }


    void InitEditorAction()
    {
        Registry::Type<EditorTransaction>();
        Registry::Type<EditorAction>();

        InitAssetEditorActions();
    }
}
