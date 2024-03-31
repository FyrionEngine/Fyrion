#include <iostream>
#include "Sinks.hpp"

namespace Fyrion
{

    void StdOutSink::SetLevel(LogLevel level)
    {
        m_logLevel = level;
    }

    bool StdOutSink::CanLog(LogLevel level)
    {
        return m_logLevel < level;
    }

    void StdOutSink::DoLog(LogLevel level, const StringView& logName, const StringView& message)
    {
        std::cout << std::string_view(message.CStr(), message.Size()) << std::endl;
    }
}
