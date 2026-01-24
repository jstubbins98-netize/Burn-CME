cmake_minimum_required(VERSION 3.16)
project(BurnCME VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(OPENGL_PKG gl)
    if(OPENGL_PKG_FOUND)
        set(OPENGL_INCLUDE_DIR ${OPENGL_PKG_INCLUDE_DIRS})
        set(OPENGL_opengl_LIBRARY ${OPENGL_PKG_LINK_LIBRARIES})
        set(OPENGL_glx_LIBRARY ${OPENGL_PKG_LINK_LIBRARIES})
        set(OpenGL_FOUND TRUE)
    endif()
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets Core)

add_executable(burn-cme-gui
    src/GUI/gui_main.cpp
    src/GUI/mainwindow.cpp
    src/GUI/mainwindow.h
    src/GUI/progressdialog.h
    src/common/core.h
)

target_link_libraries(burn-cme-gui PRIVATE
    Qt6::Widgets
    Qt6::Core
)

target_include_directories(burn-cme-gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/GUI
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
)

add_executable(burn-cme-cli
    src/CLI/main.cpp
)

target_compile_options(burn-cme-cli PRIVATE -Wall -Wextra -O2)

install(TARGETS burn-cme-gui burn-cme-cli
    RUNTIME DESTINATION bin
)
