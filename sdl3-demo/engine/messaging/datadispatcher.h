#pragma once

#include <array>
#include <vector>
#include <messaging/datapump.h>

class Component;

class DataDispatcher
{
	using DataHandlerFn = void(*)(Component *, const DataPumpBase &);
	using DataHandler = std::pair<Component *, DataHandlerFn>;

	std::array<std::vector<DataHandler>, MAX_TYPES> registrations;

public:
	template<typename ComponentType, typename DPType>
	void registerHandler(Component *component)
	{
		auto &handlers = registrations[DPType::index()];
		handlers.emplace_back(component, [](Component *comp, const DataPumpBase &base) {
			static_cast<ComponentType *>(comp)->onData(static_cast<const DPType &>(base));
		});
	}

	template<typename DPType>
	void send(const DPType &dataPump)
	{
		auto &regs = registrations[DPType::index()];
		for (auto &reg : regs)
		{
			reg.second(reg.first, dataPump);
		}
	}
};
