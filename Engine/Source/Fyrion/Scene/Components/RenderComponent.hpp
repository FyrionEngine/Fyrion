#pragma once
#include "Fyrion/Resource/ResourceTypes.hpp"
#include "Fyrion/Scene/Component.hpp"


namespace Fyrion
{
    class FY_API RenderComponent : public Component
    {
    public:
        void        OnStart() override;
        static void RegisterType(NativeTypeHandler<RenderComponent>& type);
    private:
        RID m_resourceGraph;
    };
}
