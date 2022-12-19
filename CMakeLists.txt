# SPDX-License-Identifier: GPL-3.0-only

# Require at least version 3.12
cmake_minimum_required(VERSION 3.12)

# Project name - You should change this if you're making a fork
project("Six Shooter"
    VERSION 0.2.0
    DESCRIPTION "GUI frontend for Invader tools"
    LANGUAGES C CXX
)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_executable(six-shooter
    src/main.cpp
    src/main_window.cpp
    src/map_builder.cpp
    src/map_extractor.cpp
    src/settings_editor.cpp
    src/console_box.cpp
    src/tag_bludgeoner.cpp
    src/tag_tree_widget.cpp
    src/settings.cpp
    src/tag_tree_dialog.cpp
    src/console_dialog.cpp
    src/universal.qrc
)

set(SIXSHOOTER_LIBRARIES Qt6::Widgets)

if(WIN32)
    # Add dark theming and the Windows resource file
    target_sources(six-shooter PRIVATE src/windows.rc src/theme.cpp)

    # Don't spawn a CMD window when you open Six shooter
    set(SIXSHOOTER_LIBRARIES ${SIXSHOOTER_LIBRARIES} -mwindows)

    # Required for FORTIFY_SOURCES
    if(MINGW)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fstack-protector")
        set(SIXSHOOTER_LIBRARIES ${SIXSHOOTER_LIBRARIES} ssp)
    endif()
endif()

target_link_libraries(six-shooter ${SIXSHOOTER_LIBRARIES})
target_include_directories(six-shooter PUBLIC ${Qt6Widgets_INCLUDE_DIRS})
