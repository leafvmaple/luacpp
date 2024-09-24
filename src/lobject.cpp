#include "lobject.h"
#include "luaconf.h"
#include "lstate.h"

const TValue luaO_nilobject_(nullptr);

int luaO_str2d(const char* s, lua_Number* result) {
    char* endptr = nullptr;
    *result = lua_str2number(s, &endptr);
    return true;
}
