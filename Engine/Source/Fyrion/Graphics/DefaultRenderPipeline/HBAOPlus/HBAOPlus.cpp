#include "HBAOPlus.hpp"


namespace Fyrion
{
    namespace HBAOPlus
    {
        void RegisterLinearDepthPass();
    }

    void RegisterHBAOPlus()
    {
        HBAOPlus::RegisterLinearDepthPass();
    }
}
