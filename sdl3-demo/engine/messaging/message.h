#pragma once

#include <cassert>

constexpr static int MAX_MESSAGES_TYPES = 1000;

class MessageBase
{
protected:
	static inline int nextIndex = 0;
};

template<typename MessageType>
class Message : public MessageBase
{

public:
	constexpr static int index()
	{
		assert(nextIndex < MAX_MESSAGES_TYPES && "Exceeded maximum message index limit");
		static int msgIndex = nextIndex++;
		return msgIndex;
	}
};

