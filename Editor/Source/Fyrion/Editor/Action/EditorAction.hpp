#pragma once
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Core/Pair.hpp"


namespace Fyrion
{
    class TypeHandler;

    class EditorAction
    {
    public:
        virtual ~EditorAction() = default;

        virtual void Commit() = 0;
        virtual void Rollback() = 0;
    };

    class EditorTransaction
    {
    public:
        ~EditorTransaction();

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

        void Commit();
        void Rollback();


    private:
        Array<Pair<TypeHandler*, EditorAction*>> actions;
    };
}
