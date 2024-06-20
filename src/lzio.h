#pragma once

#include "lua.h"

#define EOZ	(-1)			/* end of stream */

struct ZIO {
    size_t n = 0;
    const char* p = nullptr;
    lua_Reader reader;
    void* data;
    lua_State* L;
};

int zgetc(ZIO* z);

int luaZ_fill(ZIO* z);
