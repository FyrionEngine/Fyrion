#include <iostream>
#include "Sinks.hpp"

namespace Fyrion
{
    bool StdOutSink::CanLog(LogLevel level)
    {
        return true;
    }

    void StdOutSink::DoLog(LogLevel level, const StringView& logName, const StringView& message)
    {
        std::cout << std::string_view(message.CStr(), message.Size()) << std::endl;
    }
}
