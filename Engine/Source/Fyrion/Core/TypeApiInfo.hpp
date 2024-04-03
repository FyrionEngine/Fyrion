#pragma once

#include "Fyrion/Common.hpp"

namespace Fyrion
{
	template<typename Type>
	struct TypeApiInfo
	{
		static void ExtractApi(VoidPtr pointer)
		{

		}

		static constexpr TypeID GetApiId()
		{
			return 0;
		}
	};
}