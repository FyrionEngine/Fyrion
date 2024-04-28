#pragma once

#include "Fyrion/Common.hpp"
#include "StringView.hpp"
#include "Format.hpp"
#include "String.hpp"
#include "Array.hpp"

namespace Fyrion
{
    enum class LogLevel
    {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warn = 3,
        Error = 4,
        Critical = 5,
        Off = 6
    };

    struct LogSink
    {
        virtual ~LogSink() = default;
        virtual void SetLevel(LogLevel level) = 0;
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
            Log(LogLevel::Trace, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Debug(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel::Debug, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Info(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel::Info, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Warn(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel::Warn, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Error(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel::Error, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void Critical(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel::Critical, fmt, Traits::Forward<Args>(args)...);
        }

        template<typename ...Args>
        inline void FatalError(const fmt::format_string<Args...>& fmt, Args&& ...args)
        {
            Log(LogLevel::Error, fmt, Traits::Forward<Args>(args)...);
            FY_ASSERT(false, "error");
        }

        static Logger&  GetLogger(const StringView& name);
        static Logger&  GetLogger(const StringView& name, LogLevel logLevel);
        static void     RegisterSink(LogSink& logSink);
        static void     UnregisterSink(LogSink& logSink);
        static void     SetDefaultLevel(LogLevel logLevel);
        static void     Reset();

    private:
        String           m_name{};
        LogLevel         m_logLevel{};
        Array<LogSink*>  m_logSinks{};
    };

}