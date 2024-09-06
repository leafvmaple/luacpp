#include "lua.h"
#include "ldo.h"
#include "lfunc.h"
#include "lstate.h"
#include "lstring.h"
#include "ltable.h"
#include "lvm.h"
#include "lzio.h"

TValue* index2adr(lua_State* L, int idx) {
    if (idx > 0) {
        return L->base + (idx - 1);
    }
    else if (idx > LUA_REGISTRYINDEX) {
        return L->stack.top() + idx;
    }
    else switch (idx) {  /* pseudo-indices */
    case LUA_REGISTRYINDEX: {
        return registry(L);
    }
    case LUA_ENVIRONINDEX: {
        Closure* func = static_cast<Closure*>(L->base_ci.back().func->gc);
        L->env.setvalue(func->env);
        return &L->env;
    }
    case LUA_GLOBALSINDEX: {
        return gt(L);
    }
    default: {
        return nullptr;
    }
    }
}

// 获取当前所在环境表
Table* getcurrenv(lua_State* L) {
    if (L->base_ci.size() == 1) // 堆栈层数为1，那么为全局环境
        return static_cast<Table*>(gt(L)->gc);
}

// 将C函数压栈并调用
// |         |           |
// | Closure | UsderData | L->top
static void f_Ccall(lua_State* L, CCallS* c) {
    CClosure* cl = new (L) CClosure(L, 0, getcurrenv(L));
    cl->f = c->func;
    L->stack.emplace_back(TValue(cl, "[Call] CClosure"));
    L->stack.emplace_back(TValue(c->ud, "[Call] UsderData"));
    luaD_call(L, L->stack.top() - 2, 0); // 指向cl
}

int lua_gc(lua_State* L, int what, int data) {
    switch (what) {
    case LUA_GCCOLLECT:
        luaC_fullgc(L);
        break;
    default:
        break;
    }
    return 0;
}

void lua_pushnil(lua_State* L) {
    L->stack.emplace_back(TValue());
}

void lua_pushlstring(lua_State* L, const char* s, size_t l, _IMPL) {
    L->stack.emplace_back(TValue(strtab(L)->newlstr(L, s, l), debug ? debug : s));
}

void lua_pushstring(lua_State* L, const char* s, _IMPL) {
    if (s == nullptr)
        lua_pushnil(L);
    else
        lua_pushlstring(L, s, strlen(s), debug);
}

void lua_pushvalue(lua_State* L, int idx) {
    L->stack.push_back(*index2adr(L, idx));
}

void lua_pushcclosure(lua_State* L, lua_CFunction fn, int n, _IMPL) {
    CClosure* cl = new (L) CClosure(L, n, getcurrenv(L));
    cl->f = fn;
    while (n--)
        cl->upvalue[n] = L->stack.pop();
    L->stack.emplace_back(TValue(cl, debug));
}

void lua_pushcfunction(lua_State* L, lua_CFunction f, _IMPL) {
    lua_pushcclosure(L, f, 0, debug);
}

void lua_settable(lua_State* L, int idx) {
    Table* t = ((Table*)index2adr(L, idx)->gc);
    TValue value = L->stack.pop();
    TValue key = L->stack.pop();

    (*t)[&key] = value;
}

void lua_setfield(lua_State* L, int idx, const char* k) {
    Table* t = ((Table*)index2adr(L, idx)->gc);
    TValue key(strtab(L)->newstr(L, k), k);

    (*t)[&key] = L->stack.pop();
}

void lua_createtable(lua_State* L, int narr, int nrec, _IMPL) {
    L->stack.emplace_back(TValue(new (L) Table(L, narr, nrec), debug));
}

void* lua_touserdata(lua_State* L, int idx) {
    TValue* o = index2adr(L, idx);

    switch (o->tt) {
    case LUA_TLIGHTUSERDATA:
        return o->p;
    default:
        return nullptr;
    }
}

void lua_gettable(lua_State* L, int idx) {
    Table* t = ((Table*) index2adr(L, idx)->gc);
    TValue* key = &L->stack.back();

    *(key) = (*t)[key];
}

void lua_getfield(lua_State* L, int idx, const char* k) {
    Table* t = ((Table*)index2adr(L, idx)->gc);
    TValue key(strtab(L)->newstr(L, k));

    L->stack.push_back((*t)[&key]);
}

int lua_gettop(lua_State* L) {
    return static_cast<int>(L->stack.top() - L->base);
}

void lua_settop(lua_State* L, int idx) {
    int nSize = 0;
    if (idx >= 0)
        L->stack.settop(L->base + idx);
    else
        L->stack.expand(idx + 1);
}

void lua_pop(lua_State* L, int n) {
    lua_settop(L, -n - 1);
}

void lua_remove(lua_State* L, int idx) {
    TValue* p = index2adr(L, idx);
    L->stack.erase(p - L->stack.data() + L->stack.begin());
}

bool lua_isnil(lua_State* L, int idx) {
    return lua_type(L, idx) == LUA_TNIL;
}

bool lua_istable(lua_State* L, int idx) {
    return lua_type(L, idx) == LUA_TTABLE;
}

int lua_type(lua_State* L, int idx) {
    TValue* o = index2adr(L, idx);
    return o->tt;
}

void lua_call(lua_State* L, int nargs, int nresults) {
    TValue* func = &L->stack.back() - nargs;
    luaD_call(L, func, nresults);
}

int lua_cpcall(lua_State* L, lua_CFunction func, void* ud) {
    struct CCallS c;

    c.func = func;
    c.ud = ud;

    f_Ccall(L, &c);

    // return luaD_pcall(L, f_Ccall, &c, _savestack(L, L->top));
    return 1;

}

int lua_load(lua_State* L, lua_Reader reader, void* data, const char* chunkname) {
    ZIO z {0, nullptr, reader, data, L};
    return luaD_protectedparser(L, &z, chunkname);
}

const char* lua_tolstring(lua_State* L, int idx, size_t* len){
    TValue* o = index2adr(L, idx);
    if (!ttisstring(o))
        o->tostring(L);
    TString* ts = static_cast<TString*>(o->gc);
    if (len)
        *len = ts->s.size();
    return ts->s.c_str();
}

const char* lua_tostring(lua_State* L, int idx) {
    return lua_tolstring(L, idx, nullptr);
}