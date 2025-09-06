find_package(SDL3 REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(glm REQUIRED)

add_executable (sdl3-demo
	"sdl3-demo.cpp"
	"timer.h"
	"animation.h"
	"components/component.h"
	"ext/stb/stb_image.h" 
	"components/rendercomponent.h"
	"components/rendercomponent.cpp"
	"sdlstate.h"
	"resources.h"
	"gamestate.h"
	"resources.cpp"
	"components/animationcomponent.h"
	"components/animationcomponent.cpp"
	"components/observer.h"
	"components/inputcomponent.h"
	"components/inputcomponent.cpp"
	"framecontext.h"
	"inputstate.h"
	"components/physicscomponent.h"
	"components/physicscomponent.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sdl3-demo PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(sdl3-demo PRIVATE SDL3::SDL3 SDL3_mixer::SDL3_mixer glm::glm)
target_include_directories(sdl3-demo PRIVATE "ext/")
