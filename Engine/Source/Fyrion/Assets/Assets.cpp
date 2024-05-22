#include "AssetTypes.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{
    void RegisterFontAsset();
    void RegisterShaderAsset();


    void RegisterAssetTypes()
    {
        ResourceTypeBuilder<GraphNodeValue>::Builder()
            .Value<GraphNodeValue::Name, String>("NodeType")
            .Value<GraphNodeValue::PublicValue, bool>("PublicValue")
            .Value<GraphNodeValue::Type, String>("Type")
            .Value<GraphNodeValue::ResourceType, String>("ResourceType")
            .SubObject<GraphNodeValue::Value>("Value")
            .Build();

        ResourceTypeBuilder<GraphNodeLinkAsset>::Builder()
            .Value<GraphNodeLinkAsset::InputNode, RID>("NodeType")
            .Value<GraphNodeLinkAsset::InputPin, String>("InputPin")
            .Value<GraphNodeLinkAsset::OutputNode, RID>("OutputNode")
            .Value<GraphNodeLinkAsset::OutputPin, String>("OutputPin")
            .Build();

        ResourceTypeBuilder<GraphNodeAsset>::Builder()
            .Value<GraphNodeAsset::NodeFunction, String>("NodeFunction")
            .Value<GraphNodeAsset::NodeOutput, String>("NodeOutput")
            .Value<GraphNodeAsset::Position, Vec2>("Position")
            .SubObjectSet<GraphNodeAsset::InputValues>("InputValues")
            .Value<GraphNodeAsset::Label, String>("Label")
            .Build();

        ResourceTypeBuilder<GraphAsset>::Builder("Fyrion::ResourceGraph")
            .SubObjectSet<GraphAsset::Links>("Links")
            .SubObjectSet<GraphAsset::Nodes>("Nodes")
            .Build();

        ResourceTypeBuilder<DCCMesh>::Builder()
            .Build();
    }

    void RegisterAssets()
    {
        RegisterFontAsset();
        RegisterShaderAsset();
    }
}
