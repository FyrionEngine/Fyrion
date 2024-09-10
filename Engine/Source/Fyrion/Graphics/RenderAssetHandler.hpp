#pragma once



namespace Fyrion
{
    struct RenderAssetHandler
    {
        virtual ~RenderAssetHandler() = default;
        virtual void RefIncrease() = 0;
        virtual void RefDecrese() = 0;
    };
}