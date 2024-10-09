#pragma once

#include "StringView.hpp"
#include "UUID.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion::Repository
{
    FY_API void    LoadPackage(StringView path);
    FY_API VoidPtr Load(UUID rid);
}
