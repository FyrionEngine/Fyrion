#pragma once

#include "Fyrion/Common.hpp"
#include "StringView.hpp"
#include "Format.hpp"
#include "String.hpp"
#include "Array.hpp"

namespace Fyrion
{
    enum LogLevel_
    {
        LogLevel_Trace = 0,
        LogLevel_Debug = 1,
        LogLevel_Info = 2,
        LogLevel_Warn = 3,
        LogLevel_Error = 4,
        LogLevel_Critical = 5,
        LogLevel_Off = 6
    };

    typedef u32 LogLevel;

    struct LogSink
    {
        virtual bool CanLog(LogLevel level) = 0;
        virtual void DoLog(LogLevel level, const StringView& logName, const StringView& message) = 0;
    };

    class FY_API Logger
    {
    private:
        Logger(const StringView& name, LogLevel logLevel);
    public:

        void PrintLog(LogLevel level, const StringView& message);
        void SetLevel(LogLevel level);
        void AddSink(LogSink& logSink);
        bool CanLog(LogLevel level);

        template<typename ...Args>
        inline void Log(LogLevel level, const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            if (!CanLog(level))
            {
                return;
            }

            usize size = fmt::formatted_size(fmt, Traits::Forward<Args>(args)...);
            BufferString<128> message(size);
            fmt::format_to(message.begin(), fmt, Traits::Forward<Args>(args)...);
            PrintLog(level, message);
        }

        template<typename ...Args>
        inline void Trace(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel_Trace, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Debug(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel_Debug, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Info(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel_Info, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Warn(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel_Warn, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Error(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel_Error, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Critical(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel_Critical, fmt, Traits::Forward<Args>(args)...);
        }

        static Logger&  GetLogger(const StringView& name);
        static Logger&  GetLogger(const StringView& name, LogLevel logLevel);
        static void     RegisterSink(LogSink& logSink);
        static void     Reset();

    private:
        String           m_name{};
        LogLevel         m_logLevel{};
        Array<LogSink*>  m_logSinks{};
    };

}