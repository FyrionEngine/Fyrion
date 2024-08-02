namespace Fyrion
{

    void RegisterSceneRenderPass();
    void RegisterLightingRenderPass();

    void DefaultRenderPipelineInit()
    {
        RegisterSceneRenderPass();
        RegisterLightingRenderPass();
    }

    void DefaultRenderPipelineShutdown()
    {

    }
}
