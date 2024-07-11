#include "lfunc.h"
#include "lgc.h"

Proto::Proto(lua_State* L) {
    link(L);
}

CClosure::CClosure(lua_State* L, int nelems, Table* e) {
    upvalue.resize(nelems);
    link(L);
    env = e;
}

LClosure::LClosure(lua_State* L, int nelems, Table* e) {
    upvals.resize(nelems);
    link(L);
    env = e;
}
