find_package(SDL3 REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(glm REQUIRED)
find_package(tinyxml2 REQUIRED)

set(CMAKE_CXX_STANDARD 23)

add_executable (sdl3-demo
	"sdl3-demo.cpp"
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
	"components/component.h"
	"components/component.cpp"
	"components/animationcomponent.h"
	"components/animationcomponent.cpp"
	"components/observer.h"
	"components/inputcomponent.h"
	"components/inputcomponent.cpp"
	"components/physicscomponent.h"
	"components/physicscomponent.cpp"
	"components/collisioncomponent.h"
	"components/collisioncomponent.cpp"
 "events.h" "components/playeranimationcomponent.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sdl3-demo PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(sdl3-demo PRIVATE SDL3::SDL3 SDL3_mixer::SDL3_mixer glm::glm tinyxml2::tinyxml2)
target_include_directories(sdl3-demo PRIVATE "ext/")
