find_package(SDL3 REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(glm REQUIRED)
find_package(tinyxml2 REQUIRED)

set(CMAKE_CXX_STANDARD 23)

add_library(engine STATIC
	"tmx.cpp"
	"timer.h"
	"animation.h"
	"ext/stb/stb_image.h" 
	"components/rendercomponent.h"
	"components/rendercomponent.cpp"
	"sdlstate.h"
	"resources.h"
	"gamestate.h"
	"resources.cpp"
	"framecontext.h"
	"inputstate.h"
	"messaging/observer.h"
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
	"messaging/events.h"
	"messaging/message.h"
	"messaging/messagedispatch.h"
	"messaging/messages.h"
	"engine.h"
	"engine.cpp" "application.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET engine PROPERTY CXX_STANDARD 23)
endif()

target_link_libraries(engine PUBLIC SDL3::SDL3 SDL3_mixer::SDL3_mixer glm::glm tinyxml2::tinyxml2)
target_include_directories(engine
	PRIVATE "ext/"
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
