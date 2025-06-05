find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_mixer REQUIRED)
find_package(glm REQUIRED)
find_package(tinyxml2 REQUIRED)

add_executable (sdl3-demo "sdl3-demo.cpp" "timer.h" "animation.h" "tmx.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET sdl3-demo PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(sdl3-demo PRIVATE SDL3::SDL3 SDL3_image::SDL3_image SDL3_mixer::SDL3_mixer glm::glm tinyxml2::tinyxml2)
