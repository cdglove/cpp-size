# *****************************************************************************
# 
# CMakeLists.txt
# 
# CMake build script for cpp-size
# 
# Copyright Chris Glover 2015
# 
# Distributed under the Boost Software License, Version 1.0.
# See accompanying file LICENSE_1_0.txt or copy at
# http://www.boost.org/LICENSE_1_0.txt
# 
# *****************************************************************************

cmake_minimum_required(VERSION 3.3)

project(cpp-size)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Instruct CMake to run uic automatically when needed.
#set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets REQUIRED)
find_package(
	Boost REQUIRED
	system
	filesystem)

set(CMAKE_CXX_STANDARD 14)
file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp contrib/*.cpp)
file(GLOB_RECURSE PROJECT_HEADERS src/*.hpp contrib/*.hpp)
file(GLOB_RECURSE PROJECT_FORMS forms/*.ui)

QT5_WRAP_UI(UIS_HDRS ${PROJECT_FORMS})

add_executable(
	cpp-size WIN32 
	${PROJECT_SOURCES}
	${PROJECT_HEADERS} 
	${PROJECT_FORMS}
)

target_include_directories(
	cpp-size PUBLIC 
	"src" 
	"contrib"
)

target_include_directories(
	cpp-size SYSTEM PUBLIC 
	${Boost_INCLUDE_DIRS}
)

target_link_libraries(
	cpp-size
	Qt5::Widgets 
	Qt5::Core
	${Boost_LIBRARIES}
)