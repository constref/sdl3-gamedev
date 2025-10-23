#pragma once

#include <cassert>

constexpr static int MAX_TYPES = 1000;

class DataPumpBase
{
protected:
	static inline int nextIndex = 0;
};

template<typename DataPumpType>
class DataPump : public DataPumpBase
{

public:
	constexpr static int index()
	{
		assert(nextIndex < MAX_TYPES && "Exceeded maximum data-pump index limit");
		static int idx = nextIndex++;
		return idx;
	}
};

