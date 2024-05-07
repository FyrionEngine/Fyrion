#pragma once
#include "String.hpp"
#include "Fyrion/Resource/ResourceTypes.hpp"

namespace Fyrion
{
    struct GraphInput
    {
    };

    struct GraphOutput
    {
    };

    struct GraphNodeAsset
    {

    };

    struct GraphNodeLink
    {
        RID    inputNode;
        String inputPin;
        RID    outputNode;
        String outputPin;
    };
}
