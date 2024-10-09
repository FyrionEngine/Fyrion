#include "ResourceAssets.hpp"

#include "Fyrion/Core/Logger.hpp"

namespace Fyrion
{
    class Logger;

    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::ResourceAssets");
    }


    RID ResourceAssets::LoadAssets(StringView name, StringView path)
    {
        logger.Debug("loading assets from {} ", path);



        return {};
    }
}
