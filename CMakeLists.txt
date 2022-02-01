# SPDX-License-Identifier: GPL-3.0-only

# Require at least version 3.12
cmake_minimum_required(VERSION 3.12)

# Project name - You should change this if you're making a fork
project("Six Shooter"
    VERSION 0.1.6
    DESCRIPTION "GUI frontend for Invader tools"
    LANGUAGES C CXX
)

# Use C++17
set(CMAKE_CXX_STANDARD 17)

if(WIN32)
    option(SIX_SHOOTER_WIN32_EXE_STATIC_LINK "set whether or not to make a completely static build of all six shooter" OFF)
endif()

if(${SIX_SHOOTER_WIN32_EXE_STATIC_LINK})
    set(USE_STATIC_QT_BY_DEFAULT ON)
endif()

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
    src/console_dialog.cpp
    src/universal.qrc
)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
    target_sources(six-shooter PRIVATE src/windows.rc src/theme.cpp)
endif()

if(${SIX_SHOOTER_WIN32_EXE_STATIC_LINK})
    target_link_libraries(six-shooter qwindows qwindowsvistastyle Qt5EventDispatcherSupport Qt5WindowsUIAutomationSupport Qt5VulkanSupport Qt5AccessibilitySupport Qt5FontDatabaseSupport Qt5ThemeSupport wtsapi32)
        
    target_compile_definitions(six-shooter PRIVATE SIX_SHOOTER_WIN32_EXE_STATIC_LINK)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -lwinpthread -static-libgcc -static-libstdc++")
endif()

target_link_libraries(six-shooter Qt5::Widgets)
target_include_directories(six-shooter PUBLIC ${Qt5Widgets_INCLUDE_DIRS})
