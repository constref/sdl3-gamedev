#include "basiccameracomponent.h"

#include <node.h>
#include <framecontext.h>
#include <resources.h>
#include <messaging/commands.h>
#include <messaging/commanddispatcher.h>
#include <world.h>

BasicCameraComponent::BasicCameraComponent(Node &owner) : Component(owner, FrameStage::Gameplay)
{
	position = { 0, 0 };
}
