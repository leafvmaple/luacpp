#pragma once

#include "lobject.h"

Table* luaH_new(lua_State* L, int narray, int nhash);