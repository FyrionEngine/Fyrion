#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "Fyrion/Core/Allocator.hpp"
#include "Fyrion/Core/Event.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Sinks.hpp"

using namespace Fyrion;

int main(int argc, char** argv)
{
    AllocatorOptions flags = AllocatorOptions_DetectMemoryLeaks;

    //flags |= AllocatorOptions_CaptureStackTrace;

    MemoryGlobals::SetOptions(flags);

    StdOutSink sink{};
    Logger::RegisterSink(sink);

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    context.setOption("no-breaks", true);

    i32 res = context.run();

    Logger::Reset();
    Event::Reset();

    return res;
}