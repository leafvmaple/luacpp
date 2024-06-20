#include "lfunc.h"
#include "lgc.h"

Proto* luaF_newproto(lua_State* L) {
    Proto* p = new Proto;
    luaC_link(L, p, LUA_TPROTO);

    return p;
}

CClosure* luaF_newCclosure(lua_State* L, int nelems, Table* e) {
    CClosure* c = new CClosure;
    c->upvalue.resize(nelems);
    luaC_link(L, c, LUA_TFUNCTION);
    c->env = e;

    return c;
}

LClosure* luaF_newLclosure(lua_State* L, int nelems, Table* e) {
    LClosure* l = new LClosure;
    l->upvals.resize(nelems);
    luaC_link(L, l, LUA_TFUNCTION);
    l->env = e;

    return l;
}
