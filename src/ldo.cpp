#include "lua.h"
#include "ldo.h"
#include "lparser.h"
#include "lfunc.h"
#include "lvm.h"

// | ci->func | ci->base | ... |        | LUA_MINSTACK - 1 .. | ci->top |
// |          | L->base  | ... | L->top |
// 准备L->ci
int LClosure::precall(lua_State* L, TValue* func, int nresults) {

    L->base_ci.back().savedpc = L->savedpc;
    L->stack.resize(L->stack.size() + LUA_MINSTACK);

    auto& ci = L->base_ci.emplace_back();
    ci.func = func;
    ci.base = ci.func + 1;

    L->base = ci.base;
    L->savedpc = &p->code.front();

    execute(L, 1);

    return PCRLUA;
}

// 准备L->ci
int CClosure::precall(lua_State* L, TValue* func, int nresults) {
    int n = 0;

    L->base_ci.back().savedpc = L->savedpc;
    auto& ci = L->base_ci.emplace_back();
    ci.func = func;
    ci.base = ci.func + 1;
    ci.nresults = nresults;

    L->base = ci.base;

    n = f(L);

    luaD_poscall(L, L->stack._Unchecked_end() - n);

    return PCRC;
}

void luaD_call(lua_State* L, TValue* func, int nResults) {
    ++L->nCcalls;
    Closure* c = static_cast<Closure*>(func->gc);
    c->precall(L, func, nResults);

    L->nCcalls--;
}

int luaD_poscall(lua_State* L, TValue* firstResult) {
    auto& ci = L->base_ci.back();
    TValue* res = ci.func;
    int wanted = ci.nresults;

    L->base_ci.pop_back();
    auto& prev = L->base_ci.back();

    L->base = prev.base;
    L->savedpc = prev.savedpc;

    for (int i = wanted; i && firstResult < L->stack._Unchecked_end(); i--)
        *res++ = *firstResult++;

    L->stack.settop(res);

    return 0;
}

struct SParser {  /* data to `f_parser' */
    ZIO* z = nullptr;
    const char* name = nullptr; // chunk name
};

void f_parser(lua_State* L, SParser* p) {
    Proto* tf = luaY_parser(L, p->z, p->name);
    LClosure* cl = new (L) LClosure(L, 0, static_cast<Table*>(gt(L)->gc)); // 环境表为全局表
    cl->p = tf;
    L->stack.emplace_back(TValue(cl, p->name));
}

int luaD_protectedparser(lua_State* L, ZIO* z, const char* name) {
    SParser p {z, name};

    f_parser(L, &p);
    return 1;
}