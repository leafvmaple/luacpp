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
    L->base_ci.resize(BASIC_CI_SIZE);
    L->ci = &L->base_ci.front();

    L->stack.resize(BASIC_STACK_SIZE + EXTRA_STACK);

    L->ci->func = &L->stack.front();
    L->ci->func->setnil("#[stack_init] Function Entry#");
    L->ci->base = L->ci->func + 1;
    L->ci->top = L->ci->base + LUA_MINSTACK;

    L->base = L->ci->base;
    L->top = L->base;
}

void f_luaopen(lua_State* L) {
    stack_init(L);

    gt(L)->setvalue(new Table(L, 0, 2));
    registry(L)->setvalue(new Table(L, 0, 2));

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

    g->currentwhite.set(WHITE0BIT);
    g->currentwhite.set(FIXEDBIT);
    luaC_white(L->marked, g);

    preinit_state(L, g);
    g->rootgc.push_back(L);

    f_luaopen(L);

    return L;
}