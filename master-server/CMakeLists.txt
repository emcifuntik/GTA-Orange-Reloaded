cmake_minimum_required(VERSION 3.0)
project(master-server)

set(CMAKE_CXX_STANDARD 11)
add_definitions(-std=c++11)

set(SOURCE_FILES
        master-server.cpp
        OrangeServer.cpp
		../deps/ServerDeps/Civet/CivetServer.cpp
		../deps/ServerDeps/Civet/civetweb.c
        NetworkManager.cpp)

add_executable(master-server ${SOURCE_FILES})

set_target_properties(master-server PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/../x64/Release)

include_directories(../deps/ServerDeps/Civet/ ../shared/ ../deps/RakNet/Source/)
link_directories(/media/sf_GTA/deps/bin/)
target_link_libraries(master-server ${PROJECT_BINARY_DIR}/../deps/bin/libRakNetLibStatic.a -lpthread -ldl)
