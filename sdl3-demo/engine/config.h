#pragma once

#include <executionmode.h>

namespace Config
{
#ifdef _DEBUG
	constexpr bool IsDebugBuild = true;
#else
	constexpr bool IsDebugBuild = false;
#endif

#ifdef EXECUTION_MODE_TOOLING
	constexpr ExecutionMode CurrentExecutionMode = ExecutionMode::Tooling;
#else
	constexpr ExecutionMode CurrentExecutionMode = ExecutionMode::Standalone;
#endif

	constexpr bool IsStandaloneMode() { return CurrentExecutionMode == ExecutionMode::Standalone; }
	constexpr bool IsToolingMode() { return CurrentExecutionMode == ExecutionMode::Tooling; }

	template<typename SVal, typename TVal>
	constexpr auto ExecSelect(SVal standaloneValue, TVal toolingValue)
	{
		if constexpr (CurrentExecutionMode == ExecutionMode::Standalone)
		{
			return standaloneValue;
		}
		else
		{
			return toolingValue;
		}
	}

	template<typename DVal, typename RVal>
	constexpr auto DebugSelect(DVal debugValue, RVal releaseValue)
	{
		if constexpr (IsDebugBuild)
		{
			return debugValue;
		}
		else
		{
			return releaseValue;
		}
	}
}
