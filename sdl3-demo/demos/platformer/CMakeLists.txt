set(CMAKE_CXX_STANDARD 23)

add_executable(platformer WIN32
	"main.cpp"
	"components/playercontrollercomponent.cpp"
	"components/playercontrollercomponent.h" "platformer.h")

target_link_libraries(platformer PRIVATE engine)