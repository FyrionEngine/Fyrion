#pragma once

#include "ResourceTypes.hpp"
#include "Fyrion/Common.hpp"
#include "Fyrion/Core/StringView.hpp"

namespace Fyrion::ResourceAssets
{
    FY_API RID LoadAssets(StringView name, StringView path);
}
