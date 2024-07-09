#include "lauxlib.h"

int libsize(const luaL_Reg* l) {
    int size = 0;
    for (; l->name; l++) size++;
    return size;
}

const char* luaL_findtable(lua_State* L, int idx, const char* fname, int szhint) {
    lua_pushvalue(L, idx);
    lua_pushstring(L, fname);
    // lua_rawget(L, -2);
    // TODO
    lua_gettable(L, -2);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        lua_createtable(L, 0, szhint, fname);
        // lua_pushlstring(L, fname, e - fname);
        lua_pushstring(L, fname);
        lua_pushvalue(L, -2);
        lua_settable(L, -4);
    }
    lua_remove(L, -2);

    return nullptr;
}

// 将l数组中的所有luaL_Reg注册到l_registry["_LOADED"][libname]中
void luaI_openlib(lua_State* L, const char* libname, const luaL_Reg* l, int nup) {
    int size = libsize(l);
    luaL_findtable(L, LUA_REGISTRYINDEX, "_LOADED", 1);
    lua_getfield(L, -1, libname);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        luaL_findtable(L, LUA_GLOBALSINDEX, libname, size);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, libname);
    }
    lua_remove(L, -2); /* remove _LOADED table */
    for (; l->name; l++) {
        lua_pushcclosure(L, l->func, nup, l->name);
        lua_setfield(L, -(nup + 2), l->name);
    }
}

typedef struct LoadS {
    const char* s;
    size_t size;
} LoadS;

void luaL_register(lua_State* L, const char* libname, const luaL_Reg* l) {
    luaI_openlib(L, libname, l, 0);
}

int luaL_checkoption(lua_State* L, int arg, const char* def, const char* const lst[]) {
    // luaL_optstring
    const char* name = lua_tostring(L, arg);
    for (int i = 0; lst[i]; i++)
        if (strcmp(lst[i], name) == 0)
            return i;
    return -1;
}

const char* getS(lua_State* L, void* ud, size_t* size) {
    LoadS* ls = (LoadS*)ud;
    if (!ls->size)
        return nullptr;
    *size = ls->size;
    ls->size = 0;
    return ls->s;
}

int luaL_loadbuffer(lua_State* L, const char* buff, size_t size, const char* name) {
    LoadS ls{ buff, size };
    return lua_load(L, getS, &ls, name);
}

lua_State* luaL_newstate() {
    return lua_newstate();
}