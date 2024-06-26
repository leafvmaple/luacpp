#include "lfunc.h"
#include "lgc.h"

Proto::Proto(lua_State* L) {
    luaC_link(L, this, LUA_TPROTO);
}

CClosure::CClosure(lua_State* L, int nelems, Table* e) {
    upvalue.resize(nelems);
    luaC_link(L, this, LUA_TFUNCTION);
    env = e;

    isC = true;
}

LClosure::LClosure(lua_State* L, int nelems, Table* e) {
    upvals.resize(nelems);
    luaC_link(L, this, LUA_TFUNCTION);
    env = e;

    isC = false;
}
