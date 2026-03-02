set(CMAKE_CXX_STANDARD 23)

set(PLATFORMER_SOURCES
	"main.cpp"
	"empty.h"
	"empty.cpp"
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${PLATFORMER_SOURCES})

add_executable(empty ${PLATFORMER_SOURCES})
target_link_libraries(empty PUBLIC engine enginetooling)

# For MSVC, you often need the 'latest' flag specifically
if(MSVC)
    target_compile_options(empty PRIVATE /std:c++latest)
    target_link_options(empty PRIVATE $<$<CONFIG:Debug>:/ignore:4099>)
endif()

set_target_properties(empty PROPERTIES
    # This tells VS where the "root" of your source is for this project
    VS_PROJECT_IMPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
)

if(MSVC)
    target_link_options(empty PRIVATE $<$<CONFIG:Debug>:/ignore:4099>)
endif()

if (EMSCRIPTEN)
    message(STATUS "Configuring Emscripten-specific flags")
	target_link_options(empty PRIVATE
		"SHELL:--preload-file ${CMAKE_SOURCE_DIR}/data@/data")

	set(CMAKE_EXECUTABLE_SUFFIX ".html")
endif()

if (EXECUTION_MODE STREQUAL "TOOLING")
	target_link_libraries(empty PRIVATE enginetooling)
endif()
