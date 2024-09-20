#include "lobject.h"
#include "luaconf.h"
#include "lstate.h"

const TValue luaO_nilobject_(nullptr);

void* operator new(std::size_t size, lua_State* L) {
    global_State* g = G(L);
    g->totalbytes += size;
    return std::malloc(size);
}

void operator delete(void* p, lua_State* L) noexcept {
    // global_State* g = G(L);
    // g->totalbytes -= size;
    std::free(p);
}

int luaO_str2d(const char* s, lua_Number* result) {
    char* endptr = nullptr;
    *result = lua_str2number(s, &endptr);
    return true;
}
