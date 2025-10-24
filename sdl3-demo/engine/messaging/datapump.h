#pragma once

#include <cassert>

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
		assert(nextIndex < 25 && "Exceeded maximum data-pump index limit");
		static int idx = nextIndex++;
		return idx;
	}
};

