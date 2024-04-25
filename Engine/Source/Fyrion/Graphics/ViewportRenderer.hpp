#pragma once
#include "GraphicsTypes.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Core/Math.hpp"

namespace Fyrion
{
    class FY_API ViewportRenderer
    {
    public:
        void SetSize(const Extent& extent);

        Texture GetColorAttachmentOutput() const;
    private:
        Extent m_extent{};
        inline static Logger& logger = Logger::GetLogger("Fyrion::ViewportRenderer", LogLevel::Debug);
    };
}
