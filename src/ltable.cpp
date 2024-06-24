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

Table* luaH_new(lua_State* L, int narray, int nhash) {
    Table* t = new Table;
    luaC_link(L, t, LUA_TTABLE);

    setarrayvector(L, t, narray);
    setnodevector(L, t, nhash);

    return t;
}

TValue* newkey(lua_State* L, Table* t, const TValue* key) {
    return &t->node[*key];
}

const TValue* Table::getnum(int key) {
    if (key > 0 && key <= array.size())
        return &array[key - 1];
    TValue o((lua_Number)key);
    auto it = node.find(o);
    if (it != node.end())
        return &(*it).second;
    return luaO_nilobject;
}

const TValue* Table::getstr(const TString* key) {
    TValue o(key);
    auto it = node.find(o);
    if (it != node.end())
        return &(*it).second;
    return luaO_nilobject;
}

const TValue* Table::get(const TValue* key) {
    switch (key->tt) {
    case LUA_TNIL: return luaO_nilobject;
    case LUA_TSTRING: return getstr((TString*)key->value.gc);
    case LUA_TNUMBER: return getnum(key->value.n);
    }
}

TValue* Table::setstr(lua_State* L, const TString* key){
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
