#include "lfunc.h"
#include "lgc.h"

Proto::Proto(lua_State* L) {
    link(L);
    G(L)->totalbytes += sizeof(Proto);
}

Proto::~Proto() {
    G(L)->totalbytes -= sizeof(Proto);
}

CClosure::CClosure(lua_State* L, int nelems, Table* e) {
    upvalue.resize(nelems);
    link(L);
    G(L)->totalbytes += sizeof(CClosure);
    env = e;
}

CClosure::~CClosure() {
    G(L)->totalbytes -= sizeof(CClosure);
}

LClosure::LClosure(lua_State* L, int nelems, Table* e) {
    upvals.resize(nelems);
    link(L);
    G(L)->totalbytes += sizeof(LClosure);
    env = e;
}

LClosure::~LClosure() {
    G(L)->totalbytes -= sizeof(LClosure);
}