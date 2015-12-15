cmake_minimum_required(VERSION 3.3)

project(cpp-size)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Instruct CMake to run uic automatically when needed.
#set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets)
find_package(Boost)

set(CMAKE_CXX_STANDARD 14)
file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp contrib/*.cpp)
file(GLOB_RECURSE PROJECT_HEADERS src/*.hpp contrib/*.hpp)
file(GLOB_RECURSE PROJECT_FORMS forms/*.ui)

QT5_WRAP_UI(UIS_HDRS ${PROJECT_FORMS})

add_executable(cpp-size WIN32 ${PROJECT_SOURCES} ${PROJECT_HEADERS} ${PROJECT_FORMS})

target_include_directories(cpp-size PUBLIC "src")
target_include_directories(cpp-size PUBLIC "contrib")
target_include_directories(cpp-size SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(cpp-size ${Boost_LIBRARIES})
target_link_libraries(cpp-size Qt5::Widgets)