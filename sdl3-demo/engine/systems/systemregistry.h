#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include <systems/system.h>

class SystemRegistry
{
	std::array<std::vector<std::unique_ptr<SystemBase>>, static_cast<size_t>(FrameStage::StageCount)> systems;
	std::unordered_map<std::type_index, SystemBase *> lookupMap;

public:
	template<typename SysType>
	void registerSystem(std::unique_ptr<SysType> &&sys)
	{
		lookupMap[typeid(SysType)] = sys.get();
		systems[static_cast<size_t>(SysType::stage())].push_back(std::move(sys));
	}

	auto &getStageSystems(FrameStage stage)
	{
		return systems[static_cast<size_t>(stage)];
	}

	auto &getSystems()
	{
		return systems;
	}


	template<typename SysType>
	SysType *getSystem()
	{
		auto sysItr = lookupMap.find(typeid(SysType));
		if (sysItr == lookupMap.end())
		{
			return nullptr;
		}
		else
		{
			return static_cast<SysType *>(sysItr->second);
		}
	}

	void clear()
	{
		for (auto &stage : systems)
		{
			for (auto &sys : stage)
			{
				sys.reset();
			}
		}
	}
};