#include <components/componentstore.h>
#include <logger.h>

void ComponentStore::removeComponent(const Component &comp)
{
	auto itr = std::find(components.begin(), components.end(), &comp);
	if (itr != components.end())
	{
		components.erase(itr);
		delete &comp;
	}
	else
	{
		Logger::error(this, "Couldn't remove component, address is invalid.");
	}
}
