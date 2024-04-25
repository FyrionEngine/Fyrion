#include "ViewportRenderer.hpp"

namespace Fyrion
{
    void ViewportRenderer::SetSize(const Extent& extent)
    {
        if (m_extent.width != extent.width || m_extent.height != extent.height )
        {
            logger.Debug("viewport size resized from ({},{}) to ({},{}) ", m_extent.width, m_extent.height,  extent.width, extent.height);
            m_extent = extent;
        }
    }

    Texture ViewportRenderer::GetColorAttachmentOutput() const
    {
        return {};
    }
}
