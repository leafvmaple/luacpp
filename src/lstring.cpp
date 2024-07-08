#include "lstring.h"
#include "lstate.h"
#include "lgc.h"

TString::TString(lua_State* L, const char* str, size_t l) {
    luaC_white(marked, G(L));
    s = std::string(str, l);
}

void stringtable::resize(int newsize) {
    hash.reserve(newsize);
}

TString* stringtable::newstr(lua_State* L, const char* s) {
    return newlstr(L, s, strlen(s));
}

TString* stringtable::newlstr(lua_State* L, const char* str, size_t l) {
    GCheader* o = hash[str];
    if (!o) {
        o = new TString(L, str, l);
        hash[str] = o;
        nuse++;
    }
    return static_cast<TString*>(o);
}
