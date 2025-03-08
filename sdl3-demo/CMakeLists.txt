find_package(SDL3 REQUIRED)

add_executable (sdl3-demo "sdl3-demo.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sdl3-demo PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(sdl3-demo PRIVATE SDL3::SDL3)
