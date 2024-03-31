#include "Event.hpp"
#include "HashMap.hpp"
#include "HashSet.hpp"

namespace Fyrion
{

    struct EventFunctionData
    {
        VoidPtr         userData{};
        VoidPtr         instance{};
        FnEventCallback callback{};
    };

    bool operator==(const EventFunctionData& r, const EventFunctionData& l)
    {
        return reinterpret_cast<usize>(r.instance) == reinterpret_cast<usize>(l.instance)
            && reinterpret_cast<usize>(r.userData) == reinterpret_cast<usize>(l.userData)
            && reinterpret_cast<usize>(r.callback) == reinterpret_cast<usize>(l.callback);
    }

    template<>
    struct Hash<EventFunctionData>
    {
        constexpr static bool hasHash = true;
        static usize Value(const EventFunctionData& functionData)
        {
            return Hash<usize>::Value(reinterpret_cast<usize>(functionData.instance)) << 2 ^ Hash<usize>::Value(reinterpret_cast<usize>(functionData.callback)) >> 2;
        }
    };

    struct EventTypeData
    {
        HashSet<EventFunctionData> events{};
    };


    namespace
    {
        HashMap<TypeID, EventTypeData> events{};
    }

    void Event::Bind(TypeID typeId, VoidPtr userData, VoidPtr instance, FnEventCallback eventCallback)
    {
        auto it = events.Find(typeId);
        if (it == events.end())
        {
            it = events.Emplace(typeId, EventTypeData{}).first;
        }

        it->second.events.Emplace(EventFunctionData{
            .userData = userData,
            .instance = instance,
            .callback = eventCallback
        });
    }

    void Event::Unbind(TypeID typeId, VoidPtr userData, VoidPtr instance, FnEventCallback eventCallback)
    {
        auto it = events.Find(typeId);
        if (it != events.end())
        {
            it->second.events.Erase(EventFunctionData{
                .userData = userData,
                .instance = instance,
                .callback = eventCallback
            });
        }
    }

    usize Event::EventCount(TypeID typeId)
    {
        auto it = events.Find(typeId);
        if (it != events.end())
        {
            return it->second.events.Size();
        }
        return 0;
    }

    EventTypeData* Event::GetData(TypeID typeId)
    {
        auto it = events.Find(typeId);
        if (it == events.end())
        {
            it = events.Emplace(typeId, EventTypeData{}).first;
        }
        return &it->second;
    }

    void Event::InvokeEvents(EventTypeData* eventTypeData, VoidPtr* parameters)
    {
        for(auto& it: eventTypeData->events)
        {
            it.first.callback(it.first.userData, it.first.instance, parameters);
        }
    }

    void EventShutdown()
    {
        events.Clear();
    }
}
