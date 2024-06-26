#include "lua.h"
#include "ldo.h"
#include "lparser.h"
#include "lfunc.h"
#include "lvm.h"

// | ci->func | ci->base | ... |        | LUA_MINSTACK - 1 .. | ci->top |
// |          | L->base  | ... | L->top |
int LClosure::precall(lua_State* L, TValue* func, int nresults) {
    CallInfo* ci = nullptr;

    L->ci->savedpc = L->savedpc;

    ci = ++L->ci;
    ci->func = func;
    ci->base = ci->func + 1;
    // TODO
    ci->top = L->top + LUA_MINSTACK;

    L->base = ci->base;
    L->top = ci->top;
    L->savedpc = &p->code.front();

    return PCRLUA;
}

int CClosure::precall(lua_State* L, TValue* func, int nresults) {
    int n = 0;
    CallInfo* ci = nullptr;

    L->ci->savedpc = L->savedpc;

    ci = ++L->ci;
    ci->func = func;
    ci->base = ci->func + 1;
    ci->top = L->top + LUA_MINSTACK;
    ci->nresults = nresults;

    L->base = ci->base;
    n = f(L);
    luaD_poscall(L, L->top - n);

    return PCRC;
}

void luaD_call(lua_State* L, TValue* func, int nResults) {
    ++L->nCcalls;
    Closure* c = static_cast<Closure*>(func->value.gc);
    if (c->precall(L, func, nResults) == PCRLUA)
        luaV_execute(L, 1);

    L->nCcalls--;
}

int luaD_poscall(lua_State* L, TValue* firstResult) {
    TValue* res = nullptr;
    CallInfo* ci = nullptr;

    ci = L->ci--;
    res = ci->func;
    L->base = L->ci->base;
    L->savedpc = L->ci->savedpc;

    for (int i = ci->nresults; i != 0 && firstResult < L->top; i--)
        res++->setobj(firstResult++);
    L->top = res;
    return 0;
}

struct SParser {  /* data to `f_parser' */
    ZIO* z = nullptr;
    const char* name = nullptr;
};

void f_parser(lua_State* L, SParser* p) {
    Proto* tf = nullptr;
    LClosure* cl = nullptr;

    tf = luaY_parser(L, p->z, p->name);
    cl = new LClosure(L, 0, static_cast<Table*>(gt(L)->value.gc));
    cl->p = tf;
    L->top++->setvalue(cl);
}

int luaD_protectedparser(lua_State* L, ZIO* z, const char* name) {
    SParser p{z, name};

    f_parser(L, &p);
    return 1;
}