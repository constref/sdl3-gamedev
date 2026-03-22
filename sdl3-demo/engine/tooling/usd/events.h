#pragma once

#include <string>
#include <messaging/event.h>

namespace usd
{
class CreateStageEvent : public Event<CreateStageEvent, FrameStage::Start>
{
	std::string m_path;
public:
	CreateStageEvent(const std::string &path) : m_path(std::move(path)) {}
	auto &path() const { return m_path; }
};

class OpenStageEvent : public Event<OpenStageEvent, FrameStage::Start>
{
	std::string m_path;
public:
	OpenStageEvent(const std::string &path) : m_path(std::move(path)) {}
	auto &path() const { return m_path; }
};
	
class SaveStageEvent : public Event<SaveStageEvent, FrameStage::Start> {};

class AddLayerEvent : public Event<AddLayerEvent, FrameStage::Start>
{
	std::string m_path;
public:
	AddLayerEvent(const std::string &path) : m_path(std::move(path)) {}
	auto &path() const { return m_path; }
};
}
