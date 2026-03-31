cmake_minimum_required(VERSION 3.19)
project(QUBED LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Svg)
qt_standard_project_setup()
set(CMAKE_AUTORCC ON)

qt_add_library(QUBED STATIC
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        viewportwidget.h
        viewportwidget.cpp
        usd/usdstagemodel.h
        usd/usdstagemodel.cpp
        usd/primnode.cpp
        usd/primnode.h
        usd/treeviewstagelistener.cpp
        usd/treeviewstagelistener.h
        usd/primpicker.h
        usd/primpicker.cpp
        usd/primpicker.ui
        usd/usdstageitemdelegate.cpp
        usd/usdstageitemdelegate.h
        resources.qrc
)

target_link_libraries(QUBED
    PUBLIC
        Qt::Core
        Qt::Widgets
        Qt::Svg
        enginetooling
)

target_include_directories(QUBED
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(QUBED PRIVATE /std:c++latest)
endif()

install(TARGETS QUBED
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QUBED
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
