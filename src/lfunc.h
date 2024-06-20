#pragma once

#include "lobject.h"

Proto* luaF_newproto(lua_State* L);
// ´´½¨CClosure
CClosure* luaF_newCclosure(lua_State* L, int nelems, Table* e);
LClosure* luaF_newLclosure(lua_State* L, int nelems, Table* e);