#include "ltable.h"

#include "lobject.h"
#include "lgc.h"

// Table 相关

static void setarrayvector(lua_State* L, Table* t, int size) {
    t->array.resize(size);
}

static void setnodevector(lua_State* L, Table* t, int size) {
    t->node.reserve(size);
}

Table::Table(lua_State* _L, int narray, int nhash) {
    link(_L);
    G(L)->totalbytes += sizeof(Table);

    setarrayvector(L, this, narray);
    setnodevector(L, this, nhash);
}

Table::~Table() {
    G(L)->totalbytes -= sizeof(Table);
}

TValue* newkey(lua_State* L, Table* t, const TValue* key) {
    auto& kv = t->node[key->hash()];
    kv.first = *key;
    return &kv.second;
}

const TValue* Table::getnum(int key) const {
    if (key > 0 && key <= array.size())
        return &array[key - 1];
    auto it = node.find(key);
    if (it != node.end())
        return &(*it).second.second;
    return luaO_nilobject;
}

const TValue* Table::getstr(const TString* key) const {
    auto it = node.find(key->hash());
    if (it != node.end())
        return &(*it).second.second;
    return luaO_nilobject;
}

const TValue* Table::get(const TValue* key) const {
    switch (key->tt) {
    case LUA_TNIL: return luaO_nilobject;
    case LUA_TSTRING: return getstr(key->ts);
    case LUA_TNUMBER: return getnum(key->n);
    }
}

int Table::traverse(global_State* g) {
    if (metatable) {
        metatable->markobject(g);
    }
    for (auto& v : array)
        v.gc->trymark(g);
    for (auto& [_, p] : node) {
        if (p.second.isnil())
            /* it = node.erase(it)*/;
        else {
            p.first.markvalue(g); // 不会改变key值
            p.second.markvalue(g);
        }
    }
    return 0;
}

TValue* Table::setnum(lua_State* L, int key) {
    const TValue* p = getnum(key);
    if (p != luaO_nilobject)
        return (TValue*)p;
    TValue k((lua_Number)key);
    return newkey(L, this, &k);
}

TValue* Table::setstr(lua_State* L, const TString* key) {
    const TValue* p = getstr(key);
    if (p != luaO_nilobject)
        return (TValue*)p;
    TValue k(key);
    return newkey(L, this, &k);
}

TValue* Table::set(lua_State* L, const TValue* key) {
    const TValue* p = get(key);
    if (p != luaO_nilobject)
        return const_cast<TValue*>(p);
    return newkey(L, this, key);
}
