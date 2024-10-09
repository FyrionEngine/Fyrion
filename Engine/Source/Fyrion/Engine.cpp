#include "Engine.hpp"

#include <thread>

#include "Fyrion/Core/Logger.hpp"
#include "Fyrion/Platform/PlatformTypes.hpp"
#include "Fyrion/Platform/Platform.hpp"
#include "Fyrion/Graphics/GraphicsTypes.hpp"
#include "Fyrion/Graphics/Graphics.hpp"
#include "TypeRegister.hpp"
#include "Asset/AssetManager.hpp"
#include "Core/StaticContent.hpp"
#include "Fyrion/ImGui/ImGui.hpp"
#include "Fyrion/Core/ArgParser.hpp"
#include "IO/FileSystem.hpp"
#include "IO/Path.hpp"
#include "Resource/ResourceAssets.hpp"

namespace Fyrion
{
    void            PlatformInit();
    void            PlatformShutdown();
    void            GraphicsInit();
    void            GraphicsCreateDevice(Adapter adapter);
    RenderCommands& GraphicsBeginFrame();
    void            GraphicsEndFrame(Swapchain swapchain);
    void            GraphicsShutdown();
    void            RegistryShutdown();
    void            EventShutdown();
    void            ShaderManagerInit();
    void            ShaderManagerShutdown();
    void            InputInit();


    namespace
    {
        Logger&   logger = Logger::GetLogger("Fyrion::Engine");
        bool      running = false;
        Window    window{};
        Swapchain swapchain{};
        Vec4      clearColor = Vec4{0, 0, 0, 1};
        f64       lastTime{};
        f64       deltaTime{};
        u64       frame{0};
        ArgParser args{};

        EventHandler<OnInit>                 onInitHandler{};
        EventHandler<OnUpdate>               onUpdateHandler{};
        EventHandler<OnBeginFrame>           onBeginFrameHandler{};
        EventHandler<OnEndFrame>             onEndFrameHandler{};
        EventHandler<OnShutdown>             onShutdownHandler{};
        EventHandler<OnShutdownRequest>      onShutdownRequest{};
        EventHandler<OnRecordRenderCommands> onRecordRenderCommands{};
        EventHandler<OnSwapchainRender>      onSwapchainRender{};
    }

    void Engine::Init()
    {
        Init(0, nullptr);
    }

    void Engine::Init(i32 argc, char** argv)
    {
        args.Parse(argc, argv);

        TypeRegister();
        InputInit();
        ShaderManagerInit();
    }

    void Engine::CreateContext(const EngineContextCreation& contextCreation)
    {
        ResourceAssets::LoadAssets("Fyrion", Path::Join(FileSystem::AssetFolder(), "Fyrion"));

        PlatformInit();

        WindowFlags windowFlags = WindowFlags::SubscriveInput;
        if (contextCreation.maximize)
        {
            windowFlags |= WindowFlags::Maximized;
        }

        if (contextCreation.fullscreen)
        {
            windowFlags |= WindowFlags::Fullscreen;
        }

        GraphicsInit();
        GraphicsCreateDevice(Adapter{});

        window = Platform::CreateWindow(contextCreation.title, contextCreation.resolution, windowFlags);
        Platform::SetWindowIcon(window, StaticContent::GetImageFile("Content/Images/Logo.jpeg"));

        swapchain = Graphics::CreateSwapchain(SwapchainCreation{
            .window = window,
            .vsync = true
        });

        ImGui::Init(window, swapchain);

        onInitHandler.Invoke();

        running = true;
    }

    void Engine::Run()
    {
        logger.Info("Fyrion Engine {} Initialized", FY_VERSION);

        while (running)
        {
            f64 currentTime = Platform::GetElapsedTime();
            deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            onBeginFrameHandler.Invoke();
            Platform::ProcessEvents();

            ImGui::BeginFrame(window, deltaTime);

            if (Platform::UserRequestedClose(window))
            {
                Shutdown();
                if (running)
                {
                    Platform::SetWindowShouldClose(window, false);
                }
            }

            onUpdateHandler.Invoke(deltaTime);

            if (Extent extent = Platform::GetWindowExtent(window))
            {
                RenderCommands& cmd = GraphicsBeginFrame();
                cmd.Begin();

                onRecordRenderCommands.Invoke(cmd, deltaTime);

                RenderPass renderPass = Graphics::AcquireNextRenderPass(swapchain);

                cmd.BeginLabel("Swapchain", {0, 0, 0, 1});

                cmd.BeginRenderPass(BeginRenderPassInfo{
                    .renderPass = renderPass,
                    .clearValue = &clearColor
                });

                ViewportInfo viewportInfo{};
                viewportInfo.x = 0.;
                viewportInfo.y = 0.;
                viewportInfo.width = (f32)extent.width;
                viewportInfo.height = (f32)extent.height;
                viewportInfo.maxDepth = 0.;
                viewportInfo.minDepth = 1.;
                cmd.SetViewport(viewportInfo);
                cmd.SetScissor(Rect{.x = 0, .y = 0, .width = extent.width, .height = extent.height});

                onSwapchainRender.Invoke(cmd);

                cmd.BeginLabel("ImGui", {0, 0, 0, 1});
                ImGui::Render(cmd);
                cmd.EndLabel();

                cmd.EndRenderPass();
                cmd.EndLabel();

                cmd.End();

                GraphicsEndFrame(swapchain);
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(16));
                ImGui::EndFrame();
            }

            onEndFrameHandler.Invoke();

            frame++;
        }

        Graphics::WaitQueue();

        onShutdownHandler.Invoke();

        Graphics::DestroySwapchain(swapchain);
        Platform::DestroyWindow(window);

        GraphicsShutdown();
        PlatformShutdown();

        ImGui::ImGuiShutdown();
    }

    void Engine::Shutdown()
    {
        bool canChose = true;
        onShutdownRequest.Invoke(&canChose);
        if (canChose)
        {
            running = false;
        }
    }

    Window Engine::GetActiveWindow()
    {
        return window;
    }

    Extent Engine::GetViewportExtent()
    {
        return Platform::GetWindowExtent(window);
    }

    StringView Engine::GetArgByName(const StringView& name)
    {
        return args.Get(name);
    }

    StringView Engine::GetArgByIndex(usize i)
    {
        return args.Get(i);
    }

    bool Engine::HasArgByName(const StringView& name)
    {
        return args.Has(name);
    }

    bool Engine::IsRunning()
    {
        return running;
    }

    u64 Engine::GetFrame()
    {
        return frame;
    }

    f64 Engine::DeltaTime()
    {
        return deltaTime;
    }

    void Engine::Destroy()
    {
        ShaderManagerShutdown();
        RegistryShutdown();
        EventShutdown();
    }
}
