set(CMAKE_CXX_STANDARD 23)

add_executable(platformer
	"main.cpp"
	"events.h"
	"components/playercontrollercomponent.cpp"
	"components/playercontrollercomponent.h"
	"platformer.h" "platformer.cpp"
	"components/weaponcomponent.h" "components/weaponcomponent.cpp"
	"components/projectilecomponent.h" "components/projectilecomponent.cpp"
	"components/healthcomponent.h" "components/healthcomponent.cpp"
	"components/enemycomponent.cpp"
	"systems/playercontrolsystem.cpp"
	"systems/weaponsystem.h" "systems/weaponsystem.cpp"
	"systems/projectilesystem.h" "systems/projectilesystem.cpp"
	"systems/damagesystem.h" "systems/damagesystem.cpp"
	"systems/enemysystem.h" "systems/enemysystem.cpp")

target_link_libraries(platformer PRIVATE engine)

if (EMSCRIPTEN)
    message(STATUS "Configuring Emscripten-specific flags")
	target_link_options(platformer PRIVATE
		"SHELL:--preload-file ${CMAKE_SOURCE_DIR}/data@/data")

	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()