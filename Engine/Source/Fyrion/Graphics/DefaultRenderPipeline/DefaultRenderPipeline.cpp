namespace Fyrion
{

    void RegisterSceneRenderPass();
    void RegisterSkyboxGenRenderPass();
    void RegisterShadowMapRenderPass();
    void RegisterLightingRenderPass();
    void RegisterSkyboxBlitRenderPass();
    void RegisterPostProcessRenderPass();

    void DefaultRenderPipelineInit()
    {
        RegisterSceneRenderPass();
        RegisterSkyboxGenRenderPass();
        RegisterShadowMapRenderPass();
        RegisterLightingRenderPass();
        RegisterSkyboxBlitRenderPass();
        RegisterPostProcessRenderPass();
    }

    void DefaultRenderPipelineShutdown()
    {

    }
}
