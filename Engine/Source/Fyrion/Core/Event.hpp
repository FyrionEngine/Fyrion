#pragma once

#include "Fyrion/Common.hpp"
#include "Traits.hpp"
#include "FixedArray.hpp"

namespace Fyrion
{

    struct EventTypeData;
    typedef void(* FnEventCallback)(VoidPtr userData, VoidPtr instance, VoidPtr* parameters);

    template<usize Id, typename T>
    struct EventType
    {
        static_assert(Traits::AlwaysFalse<T>, "invalid function parameters");
    };

    template<usize Id, typename ...Args>
    struct EventType<Id, void(Args...)>
    {
        constexpr static usize id = Id;
    };

    template<auto Func, typename FuncType, typename T>
    class EventInvoker
    {
        static_assert(Traits::AlwaysFalse<T>, "invalid function parameters");
    };

    namespace Event
    {
        FY_API void                 Bind(TypeID typeId, VoidPtr userData, VoidPtr instance, FnEventCallback eventCallback);
        FY_API void                 Unbind(TypeID typeId, VoidPtr userData, VoidPtr instance, FnEventCallback eventCallback);
        FY_API usize                EventCount(TypeID typeId);
        FY_API EventTypeData*       GetData(TypeID typeId);
        FY_API void                 InvokeEvents(EventTypeData* eventTypeData, VoidPtr* parameters);

        template<typename T, auto Func>
        void Bind()
        {
            EventInvoker<Func, decltype(Func), T>::Bind(nullptr);
        }

        template<typename T, auto Func>
        void Bind(VoidPtr instance)
        {
            EventInvoker<Func, decltype(Func), T>::Bind(instance);
        }

        template<typename T, auto Func>
        void Unbind()
        {
            EventInvoker<Func, decltype(Func), T>::Unbind(nullptr);
        }

        template<typename T, auto Func>
        void Unbind(VoidPtr instance)
        {
            EventInvoker<Func, decltype(Func), T>::Unbind(instance);
        }

        template<typename T>
        FY_API usize EventCount()
        {
            return EventCount(T::id);
        }


        FY_API void Reset();
    }


    template<auto Func, usize Id, typename ...Args>
    class EventInvoker<Func, void(*)(Args...), EventType<Id, void(Args...)>>
    {
    private:

        template<typename ...Vals>
        static void Eval(VoidPtr instance, VoidPtr* params, Traits::IndexSequence<>, Vals&& ...vals)
        {
            Func(Traits::Forward<Vals>(vals)...);
        }

        template<typename T, typename ...Tp, usize I, usize... Is, typename ...Vals>
        static void Eval(VoidPtr instance, VoidPtr* params, Traits::IndexSequence<I, Is...> seq, Vals&& ...vals)
        {
            return Eval<Tp...>(instance, params, Traits::IndexSequence<Is...>(), Traits::Forward<Vals>(vals)..., *static_cast<Traits::RemoveReference<T>*>(params[I]));
        }

        static void EventCallback(VoidPtr userData, VoidPtr instance, VoidPtr* parameters)
        {
            const usize size = sizeof...(Args);
            Eval<Args...>(instance, parameters, Traits::MakeIntegerSequence<usize, size>{});
        }
    public:

        static void Bind(VoidPtr instance)
        {
            Event::Bind(Id, nullptr, instance, &EventCallback);
        }

        static void Unbind(VoidPtr instance)
        {
            Event::Unbind(Id, nullptr, instance, &EventCallback);
        }
    };

    template<auto Func, usize Id, typename Owner, typename ...Args>
    class EventInvoker<Func, void(Owner::*)(Args...), EventType<Id, void(Args...)>>
    {
    private:

        template<typename ...Vals>
        static void Eval(VoidPtr instance, VoidPtr* params, Traits::IndexSequence<>, Vals&& ...vals)
        {
            ((*static_cast<Owner*>(instance)).*Func)(Traits::Forward<Vals>(vals)...);
        }

        template<typename T, typename ...Tp, usize I, usize... Is, typename ...Vals>
        static void Eval(VoidPtr instance, VoidPtr* params, Traits::IndexSequence<I, Is...> seq, Vals&& ...vals)
        {
            return Eval<Tp...>(instance, params, Traits::IndexSequence<Is...>(), Traits::Forward<Vals>(vals)..., *static_cast<T*>(params[I]));
        }

        static void EventCallback(VoidPtr userData, VoidPtr instance, VoidPtr* parameters)
        {
            const usize size = sizeof...(Args);
            Eval<Args...>(instance, parameters, Traits::MakeIntegerSequence<usize, size>{});
        }
    public:

        static void Bind(VoidPtr instance)
        {
            Event::Bind(Id, nullptr, instance, &EventCallback);
        }

        static void Unbind(VoidPtr instance)
        {
            Event::Unbind(Id, nullptr, instance, &EventCallback);
        }
    };

    template<auto Func, usize Id, typename Owner, typename ...Args>
    class EventInvoker<Func, void(Owner::*)(Args...) const, EventType<Id, void(Args...)>>
    {
    private:

        template<typename ...Vals>
        static void Eval(VoidPtr instance, VoidPtr* params, Traits::IndexSequence<>, Vals&& ...vals)
        {
            ((*static_cast<const Owner*>(instance)).*Func)(Traits::Forward<Vals>(vals)...);
        }

        template<typename T, typename ...Tp, usize I, usize... Is, typename ...Vals>
        static void Eval(VoidPtr instance, VoidPtr* params, Traits::IndexSequence<I, Is...> seq, Vals&& ...vals)
        {
            return Eval<Tp...>(instance, params, Traits::IndexSequence<Is...>(), Traits::Forward<Vals>(vals)..., *static_cast<T*>(params[I]));
        }

        static void EventCallback(VoidPtr userData, VoidPtr instance, VoidPtr* parameters)
        {
            const usize size = sizeof...(Args);
            Eval<Args...>(instance, parameters, Traits::MakeIntegerSequence<usize, size>{});
        }
    public:

        static void Bind(VoidPtr instance)
        {
            Event::Bind(Id, nullptr, instance, &EventCallback);
        }

        static void Unbind(VoidPtr instance)
        {
            Event::Unbind(Id, nullptr, instance, &EventCallback);
        }
    };

    template<typename T>
    class EventHandler
    {
        static_assert(Traits::AlwaysFalse<T>, "invalid event type");
    };


    template<usize Id, typename ...Args>
    class EventHandler<EventType<Id, void(Args...)>>
    {
    public:
        EventHandler() : m_eventTypeData(Event::GetData(Id))
        {
        }

        void Invoke(Args... args)
        {
            FixedArray<VoidPtr, sizeof...(Args)> params = {(VoidPtr) &args...};
            Event::InvokeEvents(m_eventTypeData, params.begin());
        }
    private:
        EventTypeData* m_eventTypeData;
    };


}