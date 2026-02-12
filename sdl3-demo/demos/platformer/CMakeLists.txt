set(CMAKE_CXX_STANDARD 23)

set(PLATFORMER_SOURCES
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
	"external.h"
	"external.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PLATFORMER_SOURCES})

add_library(gamelib SHARED ${PLATFORMER_SOURCES})
target_link_libraries(gamelib PUBLIC engine)
target_compile_definitions(gamelib PRIVATE ENGINE_EXPORTS)

# For MSVC, you often need the 'latest' flag specifically
if(MSVC)
    target_compile_options(gamelib PRIVATE /std:c++latest)
    target_link_options(gamelib PRIVATE $<$<CONFIG:Debug>:/ignore:4099>)
endif()

set_target_properties(gamelib PROPERTIES
    # This tells VS where the "root" of your source is for this project
    VS_PROJECT_IMPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Executable target which just starts main
add_executable(platformer "main.cpp")
target_link_libraries(platformer PRIVATE gamelib)
set_target_properties(platformer PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

if(MSVC)
    target_link_options(platformer PRIVATE $<$<CONFIG:Debug>:/ignore:4099>)
endif()

if (EMSCRIPTEN)
    message(STATUS "Configuring Emscripten-specific flags")
	target_link_options(platformer PRIVATE
		"SHELL:--preload-file ${CMAKE_SOURCE_DIR}/data@/data")

	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()