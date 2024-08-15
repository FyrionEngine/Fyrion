#include "GraphicsTypes.hpp"
#include "Fyrion/Core/Registry.hpp"

namespace Fyrion
{
    void MeshPrimitive::RegisterType(NativeTypeHandler<MeshPrimitive>& type)
    {
        type.Field<&MeshPrimitive::firstIndex>("firstIndex");
        type.Field<&MeshPrimitive::indexCount>("indexCount");
        type.Field<&MeshPrimitive::materialIndex>("materialIndex");
    }

    void RenderGraphEdge::RegisterType(NativeTypeHandler<RenderGraphEdge>& type)
    {
        type.Field<&RenderGraphEdge::output>("output");
        type.Field<&RenderGraphEdge::nodeOutput>("nodeOutput");
        type.Field<&RenderGraphEdge::input>("input");
        type.Field<&RenderGraphEdge::nodeInput>("nodeInput");
    }

    void InterfaceVariable::RegisterType(NativeTypeHandler<InterfaceVariable>& type)
    {
        type.Field<&InterfaceVariable::location>("location");
        type.Field<&InterfaceVariable::offset>("offset");
        type.Field<&InterfaceVariable::name>("name");
        type.Field<&InterfaceVariable::format>("format");
        type.Field<&InterfaceVariable::size>("size");
    }

    void TypeDescription::RegisterType(NativeTypeHandler<TypeDescription>& type)
    {
        type.Field<&TypeDescription::name>("name");
        type.Field<&TypeDescription::type>("type");
        type.Field<&TypeDescription::size>("size");
        type.Field<&TypeDescription::offset>("offset");
        type.Field<&TypeDescription::members>("members");
    }

    void DescriptorBinding::RegisterType(NativeTypeHandler<DescriptorBinding>& type)
    {
        type.Field<&DescriptorBinding::binding>("binding");
        type.Field<&DescriptorBinding::count>("count");
        type.Field<&DescriptorBinding::name>("name");
        type.Field<&DescriptorBinding::descriptorType>("descriptorType");
        type.Field<&DescriptorBinding::renderType>("renderType");
        type.Field<&DescriptorBinding::shaderStage>("shaderStage");
        type.Field<&DescriptorBinding::viewType>("viewType");
        type.Field<&DescriptorBinding::members>("members");
        type.Field<&DescriptorBinding::size>("size");
    }

    void DescriptorLayout::RegisterType(NativeTypeHandler<DescriptorLayout>& type)
    {
        type.Field<&DescriptorLayout::set>("set");
        type.Field<&DescriptorLayout::bindings>("bindings");
    }

    void ShaderPushConstant::RegisterType(NativeTypeHandler<ShaderPushConstant>& type)
    {
        type.Field<&ShaderPushConstant::name>("name");
        type.Field<&ShaderPushConstant::offset>("offset");
        type.Field<&ShaderPushConstant::size>("size");
        type.Field<&ShaderPushConstant::stage>("stage");
    }

    void ShaderStageInfo::RegisterType(NativeTypeHandler<ShaderStageInfo>& type)
    {
        type.Field<&ShaderStageInfo::stage>("stage");
        type.Field<&ShaderStageInfo::entryPoint>("entryPoint");
        type.Field<&ShaderStageInfo::offset>("offset");
        type.Field<&ShaderStageInfo::size>("size");
    }

    void ShaderInfo::RegisterType(NativeTypeHandler<ShaderInfo>& type)
    {
       type.Field<&ShaderInfo::inputVariables>("inputVariables");
       type.Field<&ShaderInfo::outputVariables>("outputVariables");
       type.Field<&ShaderInfo::descriptors>("descriptors");
       type.Field<&ShaderInfo::pushConstants>("pushConstants");
       type.Field<&ShaderInfo::stride>("stride");
    }

}
