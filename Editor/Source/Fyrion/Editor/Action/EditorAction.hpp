#pragma once

#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/Pair.hpp"


namespace Fyrion
{
    class TypeHandler;
    class EditorTransaction;
    typedef void(*PreActionFn)(VoidPtr userData);

    class EditorAction
    {
    public:
        virtual ~EditorAction() = default;

        virtual void Commit() = 0;
        virtual void Rollback() = 0;
    };


    struct PreExecuteContext
    {
        VoidPtr     userData;
        PreActionFn action;
    };

    class FY_API EditorTransaction
    {
    public:
        virtual ~EditorTransaction();

        EditorAction* CreateAction(TypeID typeId, VoidPtr* params, TypeID* paramTypes, usize paramNum);
        void AddAction(TypeID typeId, EditorAction* action);


        template <typename T>
        T* CreateAction()
        {
            return static_cast<T*>(CreateAction(GetTypeID<T>(), nullptr, nullptr, 0));
        }

        template <typename T, typename ...Args>
        T* CreateAction(Args&& ...args)
        {
            T* action = MemoryGlobals::GetDefaultAllocator().Alloc<T>(Traits::Forward<Args>(args)...);
            AddAction(GetTypeID<T>(), action);
            return action;
        }

        void AddPreExecute(VoidPtr usarData, PreActionFn actionFn);

        virtual void Commit();
        virtual void Rollback();

    private:
        Array<Pair<TypeHandler*, EditorAction*>> actions;
        Array<PreExecuteContext> preExecute;
    };
}
