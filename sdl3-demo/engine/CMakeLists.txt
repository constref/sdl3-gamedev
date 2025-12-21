find_package(SDL3 REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(glm REQUIRED)
find_package(tinyxml2 REQUIRED)

set(CMAKE_CXX_STANDARD 23)

add_library(engine STATIC
	"tmx.cpp"
	"timer.h"
	"animation.h"
	"engine.h"
	"application.h"
	"sdlstate.h"
	"resources.h"
	"resources.cpp"
	"framecontext.h"
	"inputstate.h"
	"ext/stb/stb_image.h" 
	"components/component.h"
	"components/component.cpp"
	"components/animationcomponent.h"
	"components/animationcomponent.cpp"
	"components/inputcomponent.h"
	"components/inputcomponent.cpp"
	"components/physicscomponent.h"
	"components/physicscomponent.cpp"
	"components/collisioncomponent.h"
	"components/collisioncomponent.cpp"
	"components/basiccameracomponent.h"
	"components/basiccameracomponent.cpp"
	"components/spritecomponent.h"
	"components/spritecomponent.cpp"
	"messaging/observer.h"
	"messaging/command.h"
	"messaging/commanddispatcher.h"
	"messaging/commands.h"
	"world.h"
	"world.cpp"
	"objectholder.h"
	"node.cpp"
	"messaging/eventqueue.h"
	"messaging/events.h"
	"messaging/dispatcher.h"
	"messaging/eventdispatcher.h"
	"messaging/event.h"
	"messaging/messaging.h"
	"framestage.h"
	"components/maintainercomponent.h"
	"logger.h"
	"containers/ringbuffer.h"
	"containers/atomicringbuffer.h"
	"animationplaybackmode.h"
	"systems/system.h"
	"systems/physicssystem.h"
	"systems/physicssystem.cpp"
	"systems/inputsystem.h"
	"systems/inputsystem.cpp"
	"systems/spriterendersystem.h"
	"systems/spriterendersystem.cpp"
	"systems/spriteanimationsystem.h"
	"systems/spriteanimationsystem.cpp"
	"systems/collisionsystem.h"
	"systems/collisionsystem.cpp"
	"systems/systemregistry.h"
	"systems/basiccamerasystem.h"
	"systems/basiccamerasystem.cpp"
	"systems/context/rendercontext.h"
	"components/componentstore.h"
	"components/componentstore.cpp"
	"services.h"
	"componentsystems.h"
	"componentsystems.cpp"
	"components/componententry.h"
	"config.h"
	
	"systems/timersystem.h"
	"systems/timersystem.cpp"  "messaging/eventqueue.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET engine PROPERTY CXX_STANDARD 23)
endif()

target_link_libraries(engine PUBLIC SDL3::SDL3 SDL3_mixer::SDL3_mixer glm::glm tinyxml2::tinyxml2)
target_include_directories(engine
	PRIVATE "ext/"
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
