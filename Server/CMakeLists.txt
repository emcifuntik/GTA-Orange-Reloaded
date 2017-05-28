cmake_minimum_required(VERSION 3.0)
project(orange-server)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-std=c++11)

set(SOURCE_FILES
        API_ext.cpp
        CClientScripting.cpp
        CConfig.cpp
        CNetwork3DText.cpp
        CNetworkBlip.cpp
        CNetworkConnection.cpp
		CNetworkMasterServer.cpp
        CNetworkMarker.cpp
        CNetworkObject.cpp
        CNetworkPlayer.cpp
        CNetworkVehicle.cpp
        CRPCPlugin.cpp
        main.cpp
        Plugin.cpp
        stdafx.cpp)

add_executable(orange-server ${SOURCE_FILES})

set_target_properties(orange-server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../x64/Release/server)

include_directories(../deps/ServerDeps/yaml-cpp/include/ ../shared/ ../deps/RakNet/Source/)
link_directories(../deps/bin/)
target_link_libraries(orange-server ${PROJECT_BINARY_DIR}/../deps/bin/libRakNetDLL.so  -lpthread ${PROJECT_BINARY_DIR}/../deps/bin/libyaml-cpp.a -ldl)
