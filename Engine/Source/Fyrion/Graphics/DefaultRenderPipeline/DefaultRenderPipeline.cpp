namespace Fyrion
{

    void RegisterSceneRenderPass();
    void RegisterLightingRenderPass();
    void RegisterSkyboxRenderPass();
    void RegisterPostProcessRenderPass();

    void DefaultRenderPipelineInit()
    {
        RegisterSceneRenderPass();
        RegisterLightingRenderPass();
        RegisterSkyboxRenderPass();
        RegisterPostProcessRenderPass();
    }

    void DefaultRenderPipelineShutdown()
    {

    }
}
