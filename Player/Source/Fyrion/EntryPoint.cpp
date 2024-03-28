
#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Sinks.hpp"

using namespace Fyrion;

int main(i32 argc, char** argv)
{
    StdOutSink stdOutSink{};
    Logger::RegisterSink(stdOutSink);


    return Engine::Run(argc, argv);
}