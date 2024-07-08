#pragma once

#include "lobject.h"
#include "lstate.h"
#include "lzio.h"

enum PR_TYPE {
    PCRLUA,  /* initiated a call to a Lua function */
    PCRC,    /* did a call to a C function */
};

struct CCallS {  /* data to `f_Ccall' */
    lua_CFunction func = nullptr;
    void* ud = nullptr;
};

void luaD_call(lua_State* L, TValue* func, int nResults);
int luaD_poscall(lua_State* L, TValue* firstResult);

int luaD_protectedparser(lua_State* L, ZIO* z, const char* name);
