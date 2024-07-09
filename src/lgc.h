#pragma once

#include "lobject.h"
#include "lstate.h"

void luaC_link(lua_State* L, GCheader* o, TVALUE_TYPE tt);

void luaC_fullgc(lua_State* L);