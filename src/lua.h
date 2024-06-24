#pragma once

#include <functional>

struct lua_State;

#define LUA_VERSION     "Lua Lite 0.1.2"
#define LUA_COPYRIGHT   "Copyright (C) 2022-2024 lvmaple.com, Zohar Lee"

enum TVALUE_TYPE {
    LUA_TNONE = -1,
    LUA_TNIL,
    LUA_TBOOLEAN,
    LUA_TLIGHTUSERDATA,
    LUA_TNUMBER,
    LUA_TSTRING,
    LUA_TTABLE,
    LUA_TFUNCTION,
    LUA_TUSERDATA,
    LUA_TTHREAD,
    LAST_TAG = LUA_TTHREAD,

    LUA_TPROTO,
    NUM_TAGS = LUA_TPROTO,
    LUA_TUPVAL,
    LUA_TDEADKEY,
};

#define LUA_REGISTRYINDEX	(-10000)
#define LUA_ENVIRONINDEX	(-10001)
#define LUA_GLOBALSINDEX	(-10002)

#define _NAME char const* debug = nullptr
#define _IMPL , char const* debug

typedef std::function<int(lua_State*)> lua_CFunction;
typedef const char* (*lua_Reader) (lua_State* L, void* ud, size_t* sz);

lua_State* lua_newstate();

void lua_pushnil(lua_State* L);
void lua_pushlstring(lua_State* L, const char* s, size_t l, _NAME);
void lua_pushstring(lua_State* L, const char* s, _NAME);
void lua_pushvalue(lua_State* L, int idx);
void lua_pushcclosure(lua_State* L, lua_CFunction fn, int n, _NAME);
void lua_pushcfunction(lua_State* L, lua_CFunction f, _NAME);

void* lua_touserdata(lua_State* L, int idx);

void lua_gettable(lua_State* L, int idx);
void lua_getfield(lua_State* L, int idx, const char* k);
inline void lua_getglobal(lua_State* L, const char* k) {
    lua_getfield(L, LUA_GLOBALSINDEX, k);
}

void lua_settable(lua_State* L, int idx);
void lua_setfield(lua_State* L, int idx, const char* k);
inline void lua_setglobal(lua_State* L, const char* s) {
    lua_setfield(L, LUA_GLOBALSINDEX, s);
}

void lua_createtable(lua_State* L, int narr, int nrec, _NAME);

int lua_gettop(lua_State* L);
void lua_settop(lua_State* L, int idx);
void lua_pop(lua_State* L, int n);
void lua_remove(lua_State* L, int idx);

bool lua_isnil(lua_State* L, int idx);
bool lua_istable(lua_State* L, int idx);
int  lua_type(lua_State* L, int idx);

void lua_call(lua_State* L, int nargs, int nresults);        // 调用一个Lua函数
int  lua_cpcall(lua_State* L, lua_CFunction func, void* ud); // 调用一个C函数
int  lua_load(lua_State* L, lua_Reader reader, void* dt, const char* chunkname); // 加载代码

const char* lua_tolstring(lua_State* L, int idx, size_t* len);
const char* lua_tostring(lua_State* L, int idx);