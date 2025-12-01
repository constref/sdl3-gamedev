#pragma once

namespace Config
{
#ifdef _DEBUG
	constexpr bool IsDebugBuild = true;
#else
	constexpr bool IsDebugBuild = false;
#endif
}
