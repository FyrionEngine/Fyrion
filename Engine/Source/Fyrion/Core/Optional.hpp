#pragma once

#include <optional>

namespace Fyrion
{
    template<typename T>
    class Optional : public std::optional<T>
    {
    };


    template<typename T, typename ...Args>
    Optional<T> MakeOptional(Args&&... args)
    {
        return static_cast<Optional<T>>(std::make_optional<T>(Traits::Forward<Args>(args)...));
    }

}
