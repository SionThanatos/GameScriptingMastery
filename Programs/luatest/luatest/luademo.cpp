#include<iostream>
#include<lua.hpp>
using namespace std;
int main() {
	lua_State* lua = luaL_newstate();
	luaL_openlibs(lua);
	luaL_dofile(lua, "main.lua");
	lua_close(lua);
	return 0;
}
