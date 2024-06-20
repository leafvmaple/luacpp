#pragma once

#include "lstate.h"

struct luaL_Reg {
	const char* name;
	lua_CFunction func;
};

const char* luaL_findtable(lua_State* L, int idx, const char* fname, int szhint);

void luaI_openlib(lua_State* L, const char* libname, const luaL_Reg* l, int nup);
void luaL_register(lua_State* L, const char* libname, const luaL_Reg* l);

int luaL_loadbuffer(lua_State* L, const char* buff, size_t sz, const char* name);

lua_State* luaL_newstate();