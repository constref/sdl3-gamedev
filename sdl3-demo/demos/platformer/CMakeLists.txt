set(CMAKE_CXX_STANDARD 23)

add_executable(platformer
	"main.cpp"
	"components/playercontrollercomponent.cpp"
	"components/playercontrollercomponent.h"
	"platformer.h"
	"platformer.cpp" "components/weaponcomponent.h" "components/weaponcomponent.cpp" "components/projectilecomponent.h" "components/projectilecomponent.cpp"  "components/healthcomponent.h" "components/healthcomponent.cpp" "components/events.h")

target_link_libraries(platformer PRIVATE engine)