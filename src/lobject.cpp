#include "lobject.h"
#include "luaconf.h"

void setobj(TValue* desc, const TValue* src) {
    desc->value.gc = src->value.gc;
    desc->tt = src->tt;
    COPY_DEBUG_NAME(desc, src);
}

int luaO_str2d(const char* s, lua_Number* result) {
    char* endptr = nullptr;
    *result = lua_str2number(s, &endptr);
    return true;
}
