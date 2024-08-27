#pragma once


namespace Fyrion
{
    class World;


    enum class SystemExecutionStage
    {
        OnUpdate = 0,
        OnPreUpdate = 1,
        OnPostUpdate = 2
    };

    enum class SystemExecutionPolicy
    {
        //default - execute only when the simulation is activated
        Simulation = 0,

        // always execute even if the simulation is not activated or on pause.
        Update     = 1
    };

    struct SystemSetup
    {
        SystemExecutionStage  stage = SystemExecutionStage::OnUpdate;
        SystemExecutionPolicy policy = SystemExecutionPolicy::Simulation;
    };

    struct System
    {
        World* world = nullptr;
        virtual      ~System() = default;

        virtual void OnInit(SystemSetup& systemSetup) {}
        virtual void OnUpdate() {}
        virtual void OnDestroy() {}
    };
}
