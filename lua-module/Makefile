FLAGS=-fpic -c -Wall -std=c++11
INC=-I ../LuaJIT-2.1.0-beta2/src/ -I ../lua-module/mysql/include/ -I ./mysql/
SOURCES=ls_mysql.cpp lua_Blips.cpp lua_Main.cpp lua_Player.cpp lua-module.cpp luasql.cpp SResource.cpp
OBJECTS=$(SOURCES:.cpp=.o)
LIBS = -lc -lluajit-5.1 -lmysqlclient
EXECUTABLE=hello

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	g++ $(INC) $(FLAGS) $(OBJECTS) -o $@

#.cpp.o:
#	g++ -shared -Wl,-soname,lua-module.so \
    lua-module.so $< -o $@