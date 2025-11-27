#pragma once

#include <utility>
#include <vector>
#include <array>
#include <nodehandle.h>

template<typename Base, typename RecipientBase, typename Policy>
class Dispatcher
{
	using HandlerFn = void(*)(RecipientBase *, NodeHandle target, const Base &);
	using Handler = std::pair<RecipientBase *, HandlerFn>;
	using StageHandlers = std::array<std::vector<Handler>, static_cast<size_t>(FrameStage::StageCount)>;

	std::array<StageHandlers, 64> deliverables;

public:
	template<typename Type, typename RecipientType>
	void registerHandler(RecipientType *recipient)
	{
		auto &stageHandlers = deliverables[Type::typeIndex()];
		auto &handlers = stageHandlers[static_cast<size_t>(RecipientType::stage())];
		handlers.emplace_back(recipient, [](RecipientBase *to, NodeHandle target, const Base &base)
		{
			Policy::invoke(static_cast<RecipientType *>(to), target, static_cast<const Type &>(base));
		});
	}

	void unregisterHandler(const RecipientBase *recipient)
	{
		for (int i = 0; i < deliverables.size(); ++i)
		{
			auto &stageHandlers = deliverables[i];
			for (auto &stage : stageHandlers)
			{
				for (auto &handlers : stage)
				{
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
		}
	}

	template<typename Type>
	size_t getHandlerCount()
	{
		auto &stages = deliverables[Type::typeIndex()];
		size_t handlerCount = 0;
		for (auto &stage : stages)
		{
			handlerCount += stage.size();
		}
		return handlerCount;
	}

	template<typename Type>
	size_t send(NodeHandle target, const Type &obj, FrameStage stage)
	{
		auto &stageHandlers = deliverables[Type::typeIndex()];
		auto &handlerList = stageHandlers[static_cast<size_t>(stage)];

		for (auto &handler : handlerList)
		{
			handler.second(handler.first, target, obj);
		}
		return handlerList.size();
	}
};
