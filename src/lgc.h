#pragma once

#include "lobject.h"
#include "lstate.h"

void luaC_link(lua_State* L, GCheader* o, TVALUE_TYPE tt);

inline void luaC_white(lua_marked &marked, global_State* g) {
    marked.reset();
    marked[WHITE0BIT] = g->currentwhite[WHITE0BIT];
    marked[WHITE1BIT] = g->currentwhite[WHITE1BIT];
}