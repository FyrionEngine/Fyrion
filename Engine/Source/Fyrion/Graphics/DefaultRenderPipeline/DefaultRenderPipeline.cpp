namespace Fyrion
{

    void RegisterSceneRenderPass();
    void RegisterSkyboxGenRenderPass();
    void RegisterShadowMapRenderPass();
    void RegisterLightingRenderPass();
    void RegisterSkyboxBlitRenderPass();
    void RegisterPostProcessRenderPass();
    void RegisterHBAOPlus();

    void DefaultRenderPipelineInit()
    {
        RegisterSceneRenderPass();
        RegisterSkyboxGenRenderPass();
        RegisterShadowMapRenderPass();
        RegisterLightingRenderPass();
        RegisterSkyboxBlitRenderPass();
        RegisterPostProcessRenderPass();
        RegisterHBAOPlus();
    }

    void DefaultRenderPipelineShutdown()
    {

    }
}
