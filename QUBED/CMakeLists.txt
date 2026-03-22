cmake_minimum_required(VERSION 3.19)
project(QUBED LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_library(QUBED STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.h
    ${CMAKE_CURRENT_SOURCE_DIR}/mainwindow.ui
    ${CMAKE_CURRENT_SOURCE_DIR}/viewportwidget.h
    ${CMAKE_CURRENT_SOURCE_DIR}/viewportwidget.cpp
)

target_link_libraries(QUBED
    PUBLIC
        Qt::Core
        Qt::Widgets
        engine
)

target_include_directories(QUBED
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
        

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
