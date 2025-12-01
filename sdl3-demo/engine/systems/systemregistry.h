#pragma once

#include <vector>
#include <memory>
#include <systems/system.h>

class SystemRegistry
{
	std::array<std::vector<std::unique_ptr<SystemBase>>, static_cast<size_t>(FrameStage::StageCount)> systems;

public:
	template<typename SysType>
	void registerSystem(std::unique_ptr<SysType> &&sys)
	{
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
};