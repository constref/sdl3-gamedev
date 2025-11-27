#include <components/componentstore.h>
#include <logger.h>

void ComponentStore::remove(const Component &comp)
{
	auto itr = std::find(components.begin(), components.end(), &comp);
	if (itr != components.end())
	{
		comp.~Component();
		components.erase(itr);
	}
	else
	{
		Logger::error(this, "Couldn't remove component, address is invalid.");
	}
}
