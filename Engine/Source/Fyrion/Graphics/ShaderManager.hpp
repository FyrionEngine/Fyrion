#pragma once

#include "GraphicsTypes.hpp"
#include "Fyrion/Common.hpp"

namespace Fyrion::ShaderManager
{
    FY_API bool CompileShader(const ShaderCreation& shaderCreation, Array<u8>& bytes);
}
