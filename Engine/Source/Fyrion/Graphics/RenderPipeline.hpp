#pragma once
#include "GraphicsTypes.hpp"

namespace Fyrion
{
    struct RenderPipeline
    {
        virtual         ~RenderPipeline() = default;
        virtual void    Init() = 0;
        virtual void    RecordCommands(RenderCommands& cmd) = 0;
        virtual Texture GetColorOutput() = 0;
        virtual Texture GetDepthOutput() = 0;
        virtual Extent  GetViewportExtent() const = 0;
        virtual void    Resize(Extent extent) = 0;
        virtual void    SetCameraData(const CameraData& cameraData) = 0;
    };
}
