#pragma once
#include "lua.h"

#define LUA_COLIBNAME	"coroutine"
int luaopen_base(lua_State* L);

#define LUA_TABLIBNAME	"table"
int luaopen_table(lua_State* L);

void luaL_openlibs(lua_State* L);