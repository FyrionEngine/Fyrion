#include <doctest.h>
#include "Fyrion/Core/Format.hpp"
#include "Fyrion/Core/Logger.hpp"

using namespace Fyrion;

namespace
{
    typedef void(*FnLogCallback)(LogLevel level, const StringView& logName, const StringView& message);

    TEST_CASE("Core::Format")
    {
        std::string test = fmt::format("abc{}{}{}", 1, 2, 3);
        CHECK(test == "abc123");
    }

    struct TestSink : LogSink
    {
        FnLogCallback logCallback{};

        void SetLevel(LogLevel level) override
        {

        }

        bool CanLog(LogLevel level) override
        {
            return true;
        }

        void DoLog(LogLevel level, const StringView& logName, const StringView& message) override
        {
            logCallback(level, logName, message);
        }
    };

    i32 executeCount = 0;

    TEST_CASE("Core::TestBasicLog")
    {
        TestSink testSink{};
        Logger::RegisterSink(testSink);

        Logger& logger = Logger::GetLogger("TestLogger", LogLevel::Info);

        testSink.logCallback = [](LogLevel level, const StringView& logName, const StringView& message)
        {
            CHECK(level == LogLevel::Info);
            CHECK(logName == "TestLogger");
            CHECK(Contains(message, StringView{"test logger 123"}));
            executeCount++;
        };

        logger.Info("test logger {}", 123);
        CHECK(executeCount == 1);
        logger.Debug("this should not run");
        CHECK(executeCount == 1);

        Logger::UnregisterSink(testSink);
    }


}
