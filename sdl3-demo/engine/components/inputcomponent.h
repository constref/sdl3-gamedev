#pragma once

#include <DirectXMath.h>
#include <glm/glm.hpp>
#include <components/component.h>
#include <nodehandle.h>

class InputComponent : public Component
{
	glm::vec3 direction;
	DirectX::XMINT3 axes;
	DirectX::XMFLOAT2 m_mousePosition;
	DirectX::XMFLOAT2 m_mouseDelta;

public:
	InputComponent(Node &owner);

	glm::vec3 getDirection() const { return direction; }
	void setDirection(const glm::vec3 &direction) { this->direction = direction; }
	
	auto getAxes() const { return axes; }
	void setAxes(int x, int y, int z) { this->axes = DirectX::XMINT3(x, y, z); }
	
	DirectX::XMFLOAT2 mousePosition() const { return m_mousePosition; }
	void setMousePosition(DirectX::XMFLOAT2 mousePosition) { this->m_mousePosition = mousePosition; }
	DirectX::XMFLOAT2 mouseDelta() const { return m_mouseDelta; }
	void setMouseDelta(DirectX::XMFLOAT2 mouseDelta) { m_mouseDelta = mouseDelta; }
};