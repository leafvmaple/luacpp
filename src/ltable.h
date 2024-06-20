#pragma once

#include "lobject.h"

Table* luaH_new(lua_State* L, int narray, int nhash);
const TValue* luaH_getnum(Table* t, int key);
const TValue* luaH_getstr(Table* t, const TString* key);
const TValue* luaH_get(Table* t, const TValue* key);
TValue* luaH_setstr(lua_State* L, Table* t, const TString* key);
TValue* luaH_set(lua_State* L, Table* t, const TValue* key);