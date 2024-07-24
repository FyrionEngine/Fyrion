#pragma once

#include <optional>

namespace Fyrion
{
    template<typename T>
    class Optional : public std::optional<T>
    {
    public:

        Optional() : std::optional<T>()
        {
        }

        template<typename ...Args>
        Optional(Args&&... args) : std::optional<T>(Traits::Forward<Args>(args)...)
        {
        }

        Optional(const T& value) : std::optional<T>(value)
        {
        }
    };


    template<typename T, typename ...Args>
    Optional<T> MakeOptional(Args&&... args)
    {
        return Optional(Traits::Forward<Args>(args)...);
    }

}
