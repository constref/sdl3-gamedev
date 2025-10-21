set(CMAKE_CXX_STANDARD 23)

add_executable(platformer WIN32
	"main.cpp"
	"components/playercontrollercomponent.cpp"
	"components/playercontrollercomponent.h"
	"platformer.h"
	"platformer.cpp" "components/weaponcomponent.h" "components/weaponcomponent.cpp")

target_link_libraries(platformer PRIVATE engine)