#pragma once

// ** Lua Lite virtual machine

#include "lua.h"
#include "lobject.h"

int luaV_tostring(lua_State* L, TValue* obj);
void luaV_gettable(lua_State* L, const TValue* t, TValue* key, TValue* val);

void luaV_settable(lua_State* L, const TValue* t, TValue* key, TValue* val);

void luaV_execute(lua_State* L, int nexeccalls);