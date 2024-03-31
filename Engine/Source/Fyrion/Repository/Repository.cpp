#include "Repository.hpp"
#include "Fyrion/Core/Logger.hpp"

namespace Fyrion
{
    namespace
    {
        Logger& logger = Logger::GetLogger("Fyrion::Repository", LogLevel::Debug);
    }


    void Repository::CreateResourceType(const ResourceTypeCreation& resourceTypeCreation)
    {


        logger.Debug("Resource Type {} Created", resourceTypeCreation.name);
    }


    void RepositoryInit()
    {

    }

    void RepositoryShutdown()
    {

    }
}