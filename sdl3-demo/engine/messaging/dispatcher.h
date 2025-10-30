#pragma once

#include <utility>
#include <vector>
#include <array>

template<typename Base, typename RecipientBase, typename Policy>
class Dispatcher
{
	using HandlerFn = void(*)(RecipientBase *, const Base &);
	using Handler = std::pair<RecipientBase *, HandlerFn>;

	std::array<std::vector<Handler>, 25> deliverables;

public:
	template<typename Type, typename RecipientType>
	void registerHandler(RecipientType *recipient)
	{
		auto &handlers = deliverables[Type::typeIndex()];
		handlers.emplace_back(recipient, [](RecipientBase *to, const Base &base)
		{
			Policy::invoke(static_cast<RecipientType *>(to), static_cast<const Type &>(base));
		});
	}

	void unregisterHandler(const RecipientBase *recipient)
	{
		for (int i = 0; i < deliverables.size(); ++i)
		{
			auto &handlers = deliverables[i];

			if (handlers.size())
			{
				auto itr = std::find_if(handlers.begin(), handlers.end(),
					[recipient](const Handler &handler) { return handler.first == recipient; });
				if (itr != handlers.end())
				{
					handlers.erase(itr);
				}
			}
		}
	}

	template<typename Type>
	void send(const Type &obj)
	{
		auto &handlers = deliverables[Type::typeIndex()];
		for (auto &handler : handlers)
		{
			handler.second(handler.first, obj);
		}
	}
};
