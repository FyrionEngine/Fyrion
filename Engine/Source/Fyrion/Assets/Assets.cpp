#include "AssetTypes.hpp"
#include "Fyrion/Resource/Repository.hpp"

namespace Fyrion
{
    void RegisterFontAsset();
    void RegisterShaderAsset();


    void RegisterAssetTypes()
    {
        ResourceTypeBuilder<GraphNodeValue>::Builder()
            .Value<GraphNodeValue::Input, String>("NodeType")
            .SubObject<GraphNodeValue::Value>("Value")
            .Build();

        ResourceTypeBuilder<GraphNodeLinkAsset>::Builder()
            .Value<GraphNodeLinkAsset::InputNode, RID>("NodeType")
            .Value<GraphNodeLinkAsset::InputPin, String>("InputPin")
            .Value<GraphNodeLinkAsset::OutputNode, RID>("OutputNode")
            .Value<GraphNodeLinkAsset::OutputPin, String>("OutputPin")
            .Build();

        ResourceTypeBuilder<GraphNodeAsset>::Builder()
            .Value<GraphNodeAsset::NodeType, String>("NodeType")
            .Value<GraphNodeAsset::Position, Vec2>("Position")
            .SubObjectSet<GraphNodeAsset::InputValues>("InputValues")
            .Build();

        ResourceTypeBuilder<GraphNodeAsset>::Builder()
            .Value<GraphNodeAsset::NodeType, String>("NodeType")
            .Value<GraphNodeAsset::Position, Vec2>("Position")
            .Build();

    }

    void RegisterAssets()
    {
        RegisterFontAsset();
        RegisterShaderAsset();
    }
}
