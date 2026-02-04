set(CMAKE_CXX_STANDARD 23)

set(PLATFORMER_SOURCES
	"main.cpp"
	"events.h"
	"components/playercontrollercomponent.cpp"
	"components/playercontrollercomponent.h"
	"platformer.h"
	"platformer.cpp"
	"components/weaponcomponent.h"
	"components/weaponcomponent.cpp"
	"components/projectilecomponent.h"
	"components/projectilecomponent.cpp"
	"components/healthcomponent.h"
	"components/healthcomponent.cpp"
	"components/enemycomponent.cpp"
	"systems/playercontrolsystem.cpp"
	"systems/weaponsystem.h"
	"systems/weaponsystem.cpp"
	"systems/projectilesystem.h"
	"systems/projectilesystem.cpp"
	"systems/damagesystem.h"
	"systems/damagesystem.cpp"
	"systems/enemysystem.h"
	"systems/enemysystem.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PLATFORMER_SOURCES})

add_executable(platformer ${PLATFORMER_SOURCES})
target_link_libraries(platformer PRIVATE engine)

set_target_properties(platformer PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# For MSVC, you often need the 'latest' flag specifically
if(MSVC)
    target_compile_options(platformer PRIVATE /std:c++latest)
    target_link_options(platformer PRIVATE $<$<CONFIG:Debug>:/ignore:4099>)
endif()

set_target_properties(platformer PROPERTIES
    # This tells VS where the "root" of your source is for this project
    VS_PROJECT_IMPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

if (EMSCRIPTEN)
    message(STATUS "Configuring Emscripten-specific flags")
	target_link_options(platformer PRIVATE
		"SHELL:--preload-file ${CMAKE_SOURCE_DIR}/data@/data")

	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()