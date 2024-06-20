#pragma once

#include "string.h"

#include "lua.h"
#include "lobject.h"
#include "lgc.h"

void luaS_resize(lua_State* L, int newsize);
TString* luaS_newlstr(lua_State* L, const char* str, size_t l);

inline TString* luaS_new(lua_State* L, const char* s) {
    return luaS_newlstr(L, s, strlen(s));
}
inline void luaS_fix(TString* s) {
    s->marked.set(FIXEDBIT);
}