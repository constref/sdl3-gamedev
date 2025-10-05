#pragma once

#include <array>
#include <messaging/message.h>

class Component;

class MessageDispatch
{
	using MessageHandlerFn = void(*)(Component *, const MessageBase &);
	using MessageHandler = std::pair<Component *, MessageHandlerFn>;

	std::array<std::vector<MessageHandler>, MAX_MESSAGES_TYPES> registrations;

public:
	template<typename ComponentType, typename MessageType>
	void registerHandler(Component *component)
	{
		auto &handlers = registrations[MessageType::index()];
		handlers.emplace_back(component, [](Component *comp, const MessageBase &msgBase) {
			static_cast<ComponentType *>(comp)->onMessage(static_cast<const MessageType &>(msgBase));
		});
	}


	template<typename MessageType>
	void send(const MessageType &message)
	{
		auto &regs = registrations[MessageType::index()];
		for (auto &reg : regs)
		{
			reg.second(reg.first, message);
		}
	}
};