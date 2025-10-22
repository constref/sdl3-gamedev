#pragma once

#include <vector>
#include <memory>

#include <messaging/message.h>

class MessageQueue
{
	std::vector<std::unique_ptr<MessageBase>> messageLog;

	MessageQueue()
	{
		messageLog.reserve(5000);
	}

public:
	static MessageQueue &get()
	{
		static MessageQueue instance;
		return instance;
	}


	template<typename M>
	M &enqueue()
	{
		messageLog.push_back(std::make_unique<M>());
		return messageLog.back();
	}
};