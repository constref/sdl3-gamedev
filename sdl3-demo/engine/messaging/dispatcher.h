#pragma once

#include <utility>
#include <vector>
#include <array>

template<typename Base, typename RecipientBase, typename Policy>
class Dispatcher
{
	using HandlerFn = void(*)(RecipientBase *, const Base &);
	using Handler = std::pair<RecipientBase *, HandlerFn>;

	std::array<std::vector<Handler>, 25> registrations;

public:
	template<typename RecipientType, typename Type>
	void registerHandler(RecipientBase *recipient)
	{
		auto &handlers = registrations[Type::index()];
		handlers.emplace_back(recipient, [](RecipientBase *to, const Base &base)
		{
			Policy::invoke(static_cast<RecipientType *>(to), static_cast<const Type &>(base));
		});
	}

	template<typename Type>
	void send(const Type &obj)
	{
		auto &regs = registrations[Type::index()];
		for (auto &reg : regs)
		{
			reg.second(reg.first, obj);
		}
	}
};
