#pragma once

#include <utility>
#include <vector>
#include <array>
#include <nodehandle.h>

template<typename Base, typename RecipientBase, typename Policy>
class Dispatcher
{
	using HandlerFn2 = void(*)(RecipientBase *, NodeHandle target, const Base &);
	using Handler2 = std::pair<RecipientBase *, HandlerFn2>;
	using HandlerFn = void(*)(RecipientBase *, const Base &);
	using Handler = std::pair<RecipientBase *, HandlerFn>;

	std::array<std::vector<Handler>, 25> deliverables;
	std::array<std::vector<Handler2>, 25> deliverables2;

public:
	template<typename Type, typename RecipientType>
	void registerHandler2(RecipientType *recipient)
	{
		auto &handlers = deliverables2[Type::typeIndex()];
		handlers.emplace_back(recipient, [](RecipientBase *to, NodeHandle target, const Base &base)
		{
			Policy::invoke(static_cast<RecipientType *>(to), target, static_cast<const Type &>(base));
		});
	}
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
	void send2(NodeHandle target, const Type &obj)
	{
		auto &handlers = deliverables2[Type::typeIndex()];
		for (auto &handler : handlers)
		{
			handler.second(handler.first, target, obj);
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
