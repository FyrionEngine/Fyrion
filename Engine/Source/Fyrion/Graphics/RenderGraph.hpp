#pragma once
#include "GraphicsTypes.hpp"
#include "Fyrion/Asset/Asset.hpp"


namespace Fyrion
{
    class FY_API RenderGraphNode
    {
    public:
    };

    class FY_API RenderGraphPass
    {
        RenderGraphNode* node = nullptr;

        virtual void Init() {}
        virtual void Update(f64 deltaTime) {}
        virtual void Render(f64 deltaTime, const RenderCommands& cmd) {}
        virtual void Destroy() {}
        virtual void Resize(Extent newExtent) {}
        virtual      ~RenderGraphPass() = default;
    };


    class FY_API RenderGraph : public Asset
    {
    public:
        FY_BASE_TYPES(Asset);

        Extent GetViewportExtent() const;

        bool IsLoaded() const;
        void Load(Extent p_extent);
        void Resize(Extent p_extent);

        Texture GetColorOutput() const;
        Texture GetDepthOutput() const;

        StringView GetDisplayName() const override
        {
            return "Render Graph";
        }

        static void RegisterType(NativeTypeHandler<RenderGraph>& type);
        static void RegisterPass(const RenderGraphPassCreation& renderGraphPassCreation);
        static void SetRegisterSwapchainRenderEvent(bool p_registerSwapchainRenderEvent);

    private:
        Array<String>          passes;
        Array<RenderGraphEdge> edges;

        Extent extent;
        bool loaded = false;
    };
}
