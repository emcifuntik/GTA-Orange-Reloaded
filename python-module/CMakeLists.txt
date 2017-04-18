cmake_minimum_required(VERSION 3.0)
project(python-module)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-std=c++11)

set(SOURCE_FILES python-module.cpp python_Main.cpp python_3DText.cpp python_Blip.cpp python_Marker.cpp python_Object.cpp python_Pickup.cpp python_Player.cpp python_Vehicle.cpp)
add_library(python-module SHARED ${SOURCE_FILES})

set_target_properties(python-module PROPERTIES PREFIX "")
set_target_properties(python-module PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../x64/Release/server/modules)

include_directories(/usr/include/python3.4m/ ../shared/)
link_directories(/usr/lib/x86_64-linux-gnu/)
target_link_libraries(python-module python3.4m)
