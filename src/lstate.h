#pragma once

#include <vector>
#include <list>
#include <unordered_map>

#include "lobject.h"
#include "ltm.h"

#define LUA_MINSTACK     20
#define BASIC_CI_SIZE    8
#define BASIC_STACK_SIZE (2 * LUA_MINSTACK)

#define EXTRA_STACK   5

#ifdef _DEBUG
struct TValuePtr {
    TValue* ptr = nullptr;

    TValuePtr() = default;
    TValuePtr(TValue* ptr) : ptr(ptr) {}

    TValue& operator*() {
        return *ptr;
    }

    TValue* operator->() {
        return ptr;
    }

    operator TValue* () {
        return ptr;
    }

    TValuePtr& operator++() {
        ++ptr;
        return *this;
    }

    TValuePtr operator++(int) {
        TValuePtr tmp = *this;
        ++ptr;
        return tmp;
    }

    // --it;
    TValuePtr& operator--() {
        ptr--->setnil();
        return *this;
    }

    // it--;
    TValuePtr operator--(int) {
        return ptr--;
    }

    TValuePtr operator+(int n) {
        return ptr + n;
    }

    TValuePtr operator-(int n) {
        return ptr - n;
    }

    TValuePtr& operator+=(int n) {
        if (n < 0)
            return operator-=(-n);
        return *this;
    }

    TValuePtr& operator-=(int n) {
        while (n--)
            ptr--->setnil();
        return *this;
    }
};
#else
typedef TValue* TValuePtr;
#endif

struct CallInfo
{
    TValue* top  = nullptr;
    TValue* base = nullptr;
    TValue* func = nullptr;

    int nresults = 0; // 这个函数调用接受多少个返回值

    const Instruction* savedpc = nullptr; // 保存上一个PC
};

struct stringtable {
    // TODO
    lua_GCHash hash;
    lu_int32 nuse = 0;

    void resize(int newsize);
    TString* newstr(lua_State* L, const char* str);
    TString* newlstr(lua_State* L, const char* str, size_t l);
};

struct global_State
{
    stringtable strt;
    lua_Marked currentwhite;
    GC_STATE gcstate;

    std::list<GCheader*> gray;

    lua_GCHash::iterator sweepstrgc;  /* 字符串池GC当前位置*/
    lua_GCList rootgc;  /* 根GC池 */
    lua_GCList::iterator sweepgc;  /* 根GC当前位置 */

    lu_mem totalbytes = 0;  /* total bytes allocated */
    lu_mem GCthreshold = 0;
    TValue l_registry;

    lua_State* mainthread;

    TString* tmname[TM_N];  /* array with tag-method names */
};

struct lua_State : GCheader
{
    TValuePtr top = nullptr;
    TValue* base = nullptr;

    std::vector<TValue> stack;
    std::vector<CallInfo> base_ci;

    global_State* l_G          = nullptr;
    CallInfo* ci               = nullptr;
    const Instruction* savedpc = nullptr; // 当前函数的起始指令

    unsigned short nCcalls     = 0;

    TValue l_gt;  /* table of globals */
    TValue env;

    enum { t = LUA_TTHREAD };

    virtual int traverse(global_State* g);
};

// 获取全局变量表
inline TValue* gt(lua_State* L) {
    return &L->l_gt;
}

// 获取全局状态机
inline global_State* &G(lua_State* L) {
    return L->l_G;
}

inline TValue* registry(lua_State* L) {
    return &G(L)->l_registry;
}

inline stringtable* strtab(lua_State* L) {
    return &G(L)->strt;
}

void f_luaopen(lua_State* L);
