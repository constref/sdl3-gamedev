#pragma once

#include <cassert>

constexpr static int MAX_MESSAGES = 1000;

class MessageBase
{
protected:
	static inline int nextIndex = 0;
};

template<typename CommandType>
class Message : public MessageBase
{

public:
	constexpr static int index()
	{
		assert(nextIndex < MAX_MESSAGES && "Exceeded maximum message index limit");
		static int msgIndex = nextIndex++;
		return msgIndex;
	}
};

