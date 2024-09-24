#include "lstring.h"
#include "lstate.h"
#include "lgc.h"

TString::TString(lua_State* _L, const char* str, size_t l) {
    L = _L;
    marked.towhite(G(L));
    s = std::string(str, l);

    G(L)->totalbytes += sizeof(TString) + s.size();
}

TString::~TString() {
    G(L)->totalbytes -= sizeof(TString) + s.size();
}

void stringtable::resize(int newsize) {
    hash.reserve(newsize); 
}

TString* stringtable::newstr(lua_State* L, const char* s) {
    return newlstr(L, s, strlen(s));
}

TString* stringtable::newlstr(lua_State* L, const char* str, size_t l) {
    size_t step = (l >> 5) + 1;
    size_t h = l;
    for (size_t i = l; i >= step; i -= step)
        h = h ^ ((h << 5) + (h >> 2) + static_cast<unsigned char>(str[i - 1]));

    auto& list = hash[h];
    for (auto gco : list) {
        auto ts = static_cast<TString*>(gco);
        if (ts->s == str)
            return ts;
    }

    auto ts = new TString(L, str, l);
    list.emplace_back(ts);
    return ts;
}
