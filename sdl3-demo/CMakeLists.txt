add_subdirectory("engine")

set(CMAKE_CXX_STANDARD 23)

add_executable(sdl3-demo "sdl3-demo.cpp")
target_link_libraries(sdl3-demo PRIVATE engine)