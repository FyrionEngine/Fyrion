namespace Fyrion
{

    void RegisterSceneRenderPass();
    void RegisterSkyboxGenRenderPass();
    void RegisterLightingRenderPass();
    void RegisterSkyboxBlitRenderPass();
    void RegisterPostProcessRenderPass();

    void DefaultRenderPipelineInit()
    {
        RegisterSceneRenderPass();
        RegisterSkyboxGenRenderPass();
        RegisterLightingRenderPass();
        RegisterSkyboxBlitRenderPass();
        RegisterPostProcessRenderPass();
    }

    void DefaultRenderPipelineShutdown()
    {

    }
}
