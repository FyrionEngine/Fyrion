#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "Fyrion/Core/Allocator.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Sinks.hpp"

using namespace Fyrion;

int main(int argc, char** argv)
{
    //MemoryGlobals::SetOptions(AllocatorOptions_ShowErros | AllocatorOptions_Verbose);
    Logger::SetDefaultLevel(LogLevel::Warn);

    StdOutSink sink{};
    Logger::RegisterSink(sink);

    doctest::Context context;
    context.applyCommandLine(argc, argv);
    context.setOption("no-breaks", true);

    i32 res = context.run();

    HeapStats heapStats = MemoryGlobals::GetHeapStats();
    if (heapStats.totalAllocated != heapStats.totalFreed)
    {
        res += 10000;
    }
    return  res;
}