#pragma once

#include <systems/system.h>
#include <components/lightingcomponent.h>
#include <rendering/light.h>

class LightingSystem : public System<FrameStage::Render, LightingComponent>
{
public:
	constexpr static size_t MaxLights = 32;
	
    LightingSystem(Services &services);

    void update(Node &node) override;
    void beginFrame() override;
	auto &lights() { return m_lights; }
	uint32_t numLights() const { return m_lightCount; }
	size_t lightsByteSize() const;
	
private:
	std::array<Light, MaxLights> m_lights;
	uint32_t m_lightCount;
};
