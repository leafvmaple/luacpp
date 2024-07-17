#include "lstate.h"
#include "ltable.h"
#include "lstring.h"
#include "llex.h"
#include "lgc.h"

struct LG {
    lua_State    l;
    global_State g;
};

// 初始化lua_State
// 1. 初始化l_G全局状态机
// 2. 初始化全局变量表
void preinit_state(lua_State* L, global_State* g) {
    G(L) = g;
    gt(L)->setnil("#Global Table#");
}

// 1. 初始化base_ci
// 2. 初始化stack
void stack_init(lua_State* L) {
    // prevent capacity from changing
    L->base_ci.reserve(BASIC_CI_SIZE);
    L->stack.resize(BASIC_STACK_SIZE + EXTRA_STACK);

    auto& ci = L->base_ci.emplace_back();
    // ci.func = &L->stack.emplace_back(TValue("[Init] #Function Entry#"));
    ci.func = &L->stack.front();
    ci.func->setnil("[Init] #Function Entry#");

    ci.base = ci.func + 1;
    ci.top = ci.base + LUA_MINSTACK;

    L->base = ci.base;
    L->top = L->base;
}

void f_luaopen(lua_State* L) {
    stack_init(L);

    gt(L)->setvalue(new (L) Table(L, 0, 2), "#Global Table#");
    registry(L)->setvalue(new (L) Table(L, 0, 2), "Registry Table");
    strtab(L)->resize(MINSTRTABSIZE);

    luaT_init(L);
    luaX_init(L);

    //g->GCthreshold = 4 * g->totalbytes;
}

lua_State* lua_newstate() {
    lua_State* L;
    global_State* g;

    // TODO: use LUAI_EXTRASPACE
    LG* l = new LG;

    L = &l->l;
    g = &l->g;

    g->mainthread = L;
    g->currentwhite.set(WHITE0BIT);
    g->currentwhite.set(FIXEDBIT);
    L->marked.towhite(g);

    preinit_state(L, g);
    g->rootgc.emplace_front(L);

    f_luaopen(L);

    g->sweepgc = g->rootgc.begin();
    g->sweepstrgc = strtab(L)->hash.begin();

    return L;
}

int lua_State::traverse(global_State* g) {
    // TODO
    g->gray.pop_front();
    // marked.togray(); ?

    gt(this)->markvalue(g);

    TValue* lim = top;
    for (const auto& ci : base_ci)
        if (lim < top)
            lim = ci.top;

    /*for (TValue* o = &stack.front(); o < lim; o++)
        o->gc->trymark(g); */

    TValue* o = &stack.front();
    for (; o < top; o++)
        o->markvalue(g);

    for (; o <= lim; o++)
        o->setnil();

    return 0;
}
