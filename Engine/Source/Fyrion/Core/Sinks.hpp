#pragma once

#include "Logger.hpp"

namespace Fyrion
{
    class FY_API StdOutSink : public LogSink
    {
    public:
        bool CanLog(LogLevel level) override;
        void DoLog(LogLevel level, const StringView& logName, const StringView& message) override;
    };

}