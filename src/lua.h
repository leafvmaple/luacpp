#pragma once

#include <functional>

struct lua_State;

#define LUA_VERSION     "Lua Lite 0.2.0"
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

#define LUA_GCSTOP		0
#define LUA_GCRESTART		1
#define LUA_GCCOLLECT		2
#define LUA_GCCOUNT		3
#define LUA_GCCOUNTB		4
#define LUA_GCSTEP		5
#define LUA_GCSETPAUSE		6
#define LUA_GCSETSTEPMUL	7

using lua_CFunction = std::function<int(lua_State*)>;
using lua_Reader = std::function<const char*(lua_State* L, void* ud, size_t* sz)>;

lua_State* lua_newstate();

int lua_gc(lua_State* L, int what, int data);

void lua_pushnil(lua_State* L);
void lua_pushlstring(lua_State* L, const char* s, size_t l, char const* debug = nullptr);
void lua_pushstring(lua_State* L, const char* s, char const* debug = nullptr);
void lua_pushvalue(lua_State* L, int idx);
void lua_pushcclosure(lua_State* L, lua_CFunction fn, int n, char const* debug = nullptr);
void lua_pushcfunction(lua_State* L, lua_CFunction f, char const* debug = nullptr);

void* lua_touserdata(lua_State* L, int idx);

void lua_gettable(lua_State* L, int idx);
void lua_getfield(lua_State* L, int idx, const char* k);
void lua_getglobal(lua_State* L, const char* k);

void lua_settable(lua_State* L, int idx);
void lua_setfield(lua_State* L, int idx, const char* k);
void lua_setglobal(lua_State* L, const char* s);

void lua_createtable(lua_State* L, int narr, int nrec, char const* debug = nullptr);

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