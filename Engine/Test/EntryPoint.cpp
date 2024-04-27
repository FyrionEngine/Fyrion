#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#include "Fyrion/Core/Allocator.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Sinks.hpp"

using namespace Fyrion;

int main(int argc, char** argv)
{
    //MemoryGlobals::SetOptions(AllocatorOptions_ShowErros | AllocatorOptions_Verbose);

    //Logger::SetDefaultLevel(LogLevel::Trace);

    StdOutSink sink{};
    Logger::RegisterSink(sink);


    doctest::Context context;
    context.applyCommandLine(argc, argv);
    context.setOption("no-breaks", true);

    i32 res = context.run();

    Logger::Reset();

    HeapStats heapStats = MemoryGlobals::GetHeapStats();
    if (heapStats.totalAllocated != heapStats.totalFreed)
    {
        std::cout << "Leak detected " << heapStats.totalAllocated - heapStats.totalFreed << " bytes not freed" << std::endl;
        //res += 10000;
    }
    return  res;
}