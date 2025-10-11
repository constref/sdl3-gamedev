set(CMAKE_CXX_STANDARD 23)

add_executable(platformer "platformer.cpp")
target_link_libraries(platformer PRIVATE engine)