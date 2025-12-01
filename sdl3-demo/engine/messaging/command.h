#pragma once

#include <cassert>

class CommandBase
{
protected:
	static inline int nextIndex = 0;
};

template<typename CommandType>
class Command : public CommandBase
{

public:
	constexpr static int typeIndex()
	{
		assert(nextIndex < 25 && "Exceeded maximum data-pump index limit");
		static int idx = nextIndex++;
		return idx;
	}
};

