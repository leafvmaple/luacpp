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

struct CallInfo
{
    TValue* base = nullptr;
    TValue* func = nullptr;

    int nresults = 0; // ����������ý��ܶ��ٸ�����ֵ

    const Instruction* savedpc = nullptr; // ������һ��PC
};

template<typename T>
struct lua_stack : std::vector<T> {
    void settop(const T* top) {
        // ΪʲôҪ��this->��
        this->resize(top - &this->front());
    }
    // TODO
    auto pop() {
        auto ret = this->back();
        this->pop_back();
        return ret;
    }
    /*void end() {
        return this->_Unchecked_end();
    }*/
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

    lua_GCHash::iterator sweepstrgc;  /* �ַ�����GC��ǰλ��*/
    lua_GCList rootgc;  /* ��GC�� */
    lua_GCList::iterator sweepgc;  /* ��GC��ǰλ�� */

    lu_mem totalbytes = 0;  /* total bytes allocated */
    lu_mem GCthreshold = 0;
    TValue l_registry;

    lua_State* mainthread;

    TString* tmname[TM_N];  /* array with tag-method names */
};

struct lua_State : GCheader
{
    TValue* base = nullptr;

    lua_stack<TValue> stack;
    lua_stack<CallInfo> base_ci;

    global_State* l_G          = nullptr;
    const Instruction* savedpc = nullptr; // ��ǰ��������ʼָ��

    unsigned short nCcalls     = 0;

    TValue l_gt;  /* table of globals */
    TValue env;

    enum { t = LUA_TTHREAD };

    virtual int traverse(global_State* g);
};

// ��ȡȫ�ֱ�����
inline TValue* gt(lua_State* L) {
    return &L->l_gt;
}

// ��ȡȫ��״̬��
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
