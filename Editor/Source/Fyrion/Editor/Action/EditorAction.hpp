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

    class EditorTransaction
    {
    public:
        virtual ~EditorTransaction();

        EditorAction* CreateAction(TypeID typeId, VoidPtr* params, TypeID* paramTypes, usize paramNum);

        template <typename T>
        T* CreateAction()
        {
            return static_cast<T*>(CreateAction(GetTypeID<T>(), nullptr, nullptr, 0));
        }

        template <typename T, typename ...Args>
        T* CreateAction(Args&& ...args)
        {
            TypeID ids[] = {GetTypeID<Args>()...,};
            VoidPtr params[] = {&args...};
            return static_cast<T*>(CreateAction(GetTypeID<T>(), params, ids, sizeof...(Args)));
        }

        void AddPreExecute(VoidPtr usarData, PreActionFn actionFn);

        virtual void Commit();
        virtual void Rollback();

    private:
        Array<Pair<TypeHandler*, EditorAction*>> actions;
        Array<PreExecuteContext> preExecute;
    };
}
