cmake_minimum_required(VERSION 3.0)
project(lua-module)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-std=c++11)

set(SOURCE_FILES ls_mysql.cpp lua-module.cpp lua_Main.cpp lua_Blips.cpp lua_Markers.cpp lua_Object.cpp lua_Player.cpp lua_Vehicle.cpp luasql.cpp SResource.cpp)
add_library(lua-module SHARED ${SOURCE_FILES})

set_target_properties(lua-module PROPERTIES PREFIX "")
set_target_properties(lua-module PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../x64/Release/server/modules)

include_directories(../shared/ ./mysql/include/ ../deps/ServerDeps/LuaJIT-2.1.0-beta2/src/)
target_link_libraries(lua-module mysqlclient ${PROJECT_BINARY_DIR}/../deps/bin/libluajit.so)
