find_package(SDL3 REQUIRED)
#find_package(SDL3_mixer REQUIRED)
find_package(glm REQUIRED)
find_package(tinyxml2 REQUIRED)
find_package(Vulkan REQUIRED COMPONENTS volk shaderc_combined)
find_package(flatbuffers REQUIRED)
find_package(DirectX-Headers REQUIRED)

set(CMAKE_CXX_STANDARD 23)

set(ENGINE_SOURCES
	"tmx.cpp"
	"timer.h"
	"animation.h"
	"engine.h"
	"engine.cpp"
	"bootstrap.h"
	"bootstrap.cpp"
	"application.h"
	"executionmode.h"
	"sdlstate.h"
	"resources.h"
	"resources.cpp"
	"framecontext.h"
	"inputstate.h"
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
	"components/cameracomponent.h"
	"components/cameracomponent.cpp"
	"components/basiccameracomponent.h"
	"components/basiccameracomponent.cpp"
	"components/spritecomponent.h"
	"components/spritecomponent.cpp"
	"components/meshcomponent.h"
	"components/meshcomponent.cpp"
	"messaging/observer.h"
	"messaging/command.h"
	"messaging/commanddispatcher.h"
	"messaging/commands.h"
	"world.h"
	"world.cpp"
	"objectholder.h"
	"nodehandle.h"
	"node.h"
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
	"systems/fpscamerasystem.h"
	"systems/fpscamerasystem.cpp"
	"systems/basiccamerasystem.h"
	"systems/basiccamerasystem.cpp"
	"systems/vulkanrendersystem.cpp"
	"systems/vulkanrendersystem.h"
	"systems/d3d11rendersystem.h"
	"systems/d3d11rendersystem.cpp"
	"systems/d3d12/d3d12rendersystem.h"
	"systems/d3d12/d3d12rendersystem.cpp"
	"systems/context/rendercontext.h"
	"components/componentstore.h"
	"components/componentstore.cpp"
	"services.h"
	"componentsystems.h"
	"componentsystems.cpp"
	"components/componententry.h"
	"config.h"
	"systems/timersystem.h"
	"systems/timersystem.cpp"
	"messaging/eventqueue.cpp"
	"resourceid.h"
	"resourceloader.h"
	"resourceloader.cpp"
	"prototypeid.h"
	"prototypeinstancer.h"
	"prototypeinstancer.cpp"
	"ext/tiny_gltf.cc"
	"containers/atomicringbuffer.h"
	"containers/ringbuffer.h"
	"tooling/platformutils.h"
	"tooling/platformutils.cpp"
	"tooling/engineworker.h"
	"tooling/engineworker.cpp"
	"tooling/platformevents.h"
)

set(SHADER_SOURCES
	"shaders/shader.vert"
	"shaders/shader.frag"
	"shaders/sprite.vert"
	"shaders/sprite.frag"
	"shaders/hlsl/basic.hlsl"
)

set_source_files_properties(${SHADER_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ENGINE_SOURCES} ${SHADER_SOURCES})

add_library(engine STATIC ${ENGINE_SOURCES} ${SHADER_SOURCES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET engine PROPERTY CXX_STANDARD 23)
endif()

target_compile_definitions(engine PRIVATE TINYGLTF_NO_STB_IMAGE)
if (EXECUTION_MODE STREQUAL "TOOLING")
	message(STATUS "Configuring Tooling Build")
	target_compile_definitions(engine PUBLIC EXECUTION_MODE_TOOLING)
endif()


# For MSVC, you often need the 'latest' flag specifically
if(MSVC)
    target_compile_options(engine PRIVATE /std:c++latest)
    target_link_options(engine PRIVATE $<$<CONFIG:Debug>:/ignore:4099>)
endif()

set(FLATBUFF_DIR ${CMAKE_SOURCE_DIR}/flatbuffers)
message(STATUS ${FLATBUFF_DIR})

target_link_libraries(engine PUBLIC
	Vulkan::Vulkan
	Vulkan::volk
	Vulkan::shaderc_combined
	SDL3::SDL3
	glm::glm
	tinyxml2::tinyxml2
	flatbuffers::flatbuffers
	dxgi d3d11 d3d12 d3dcompiler
	Microsoft::DirectX-Headers
)

target_include_directories(engine
	PRIVATE "ext/"
	PUBLIC
	${FLATBUFF_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

# tooling specific
#if (EXECUTION_MODE STREQUAL "TOOLING")
    find_package(pxr REQUIRED)

    message(STATUS "USD Libs: ${PXR_LIBRARIES}")
    message(STATUS "USD Includes: ${PXR_INCLUDE_DIRS}")

    add_library(enginetooling
        "tooling/systems/editorinputsystem.h"
        "tooling/systems/editorinputsystem.cpp"
        "tooling/events.h"
        "tooling/usd/usdprocessor.h"
        "tooling/usd/usdprocessor.cpp")

    target_link_libraries(enginetooling
        PRIVATE engine usd usdGeom usdImaging hd)
#endif()
