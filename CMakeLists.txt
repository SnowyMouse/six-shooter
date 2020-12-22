# SPDX-License-Identifier: GPL-3.0-only

# Require at least version 3.12
cmake_minimum_required(VERSION 3.12)

# Project name - You should change this if you're making a fork
project("Six Shooter"
    VERSION 0.1.0
    DESCRIPTION "GUI frontend for Invader tools"
    LANGUAGES C CXX
)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

# Set up C++
find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
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
    src/universal.qrc
)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
    target_sources(six-shooter PRIVATE src/windows.rc)
endif()

target_link_libraries(six-shooter Qt5::Widgets)
target_include_directories(six-shooter PUBLIC ${Qt5Widgets_INCLUDE_DIRS})
