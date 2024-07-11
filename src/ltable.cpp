#include "ltable.h"

#include "lobject.h"
#include "lgc.h"

// Table Ïà¹Ø

void setarrayvector(lua_State* L, Table* t, int size) {
    t->array.resize(size);
}

void setnodevector(lua_State* L, Table* t, int size) {
    t->node.reserve(size);
}

Table::Table(lua_State* _L, int narray, int nhash) {
    L = _L;
    link(L);

    setarrayvector(L, this, narray);
    setnodevector(L, this, nhash);
}

TValue* newkey(lua_State* L, Table* t, const TValue* key) {
    return &t->node[*key];
}

const TValue* Table::getnum(int key) const {
    if (key > 0 && key <= array.size())
        return &array[key - 1];
    TValue o((lua_Number)key);
    auto it = node.find(o);
    if (it != node.end())
        return &(*it).second;
    return luaO_nilobject;
}

const TValue* Table::getstr(const TString* key) const {
    TValue o(key);
    auto it = node.find(o);
    if (it != node.end())
        return &(*it).second;
    return luaO_nilobject;
}

const TValue* Table::get(const TValue* key) const {
    switch (key->tt) {
    case LUA_TNIL: return luaO_nilobject;
    case LUA_TSTRING: return getstr((TString*)key->value.gc);
    case LUA_TNUMBER: return getnum(key->value.n);
    }
}

int Table::traverse(global_State* g) {
    for (auto& v : array)
        v.value.gc->traverse(g);
    for (auto it = node.begin(); it != node.end();) {
        if (ttisnil(&it->second))
            it = node.erase(it);
        else {
            it->second.value.gc->traverse(g);
            ++it;
        }
    }
    return 0;
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
