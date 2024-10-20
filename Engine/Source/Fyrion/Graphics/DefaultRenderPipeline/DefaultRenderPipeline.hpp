#pragma once
#include "Fyrion/Graphics/RenderPipeline.hpp"


namespace Fyrion
{
    class DefaultRenderPipeline : public RenderPipeline
    {
    public:
        void Init() override {}

        void RecordCommands(RenderCommands& cmd) override {}

        Texture GetColorOutput() override
        {
            return {};
        }

        Texture GetDepthOutput() override
        {
            return {};
        }

        Extent GetViewportExtent() const override
        {
            return {};
        }

        void Resize(Extent extent) override {}

        void SetCameraData(const CameraData& cameraData) override {}
    };
}
