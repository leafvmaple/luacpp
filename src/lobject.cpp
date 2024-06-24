#include "lobject.h"
#include "luaconf.h"

void setnilvalue(TValue* obj _IMPL) {
    obj->tt = LUA_TNIL;
    SET_DEBUG_NAME(obj, debug);
}

void setnvalue(TValue* obj, const lua_Number n _IMPL) {
    obj->setvalue(n);
    SET_DEBUG_NAME(obj, debug);
}

void setpvalue(TValue* obj, void* p _IMPL) {
    obj->setvalue(p);
    SET_DEBUG_NAME(obj, debug);
}

void setbvalue(TValue* obj, const bool b _IMPL) {
    obj->setvalue(b);
    SET_DEBUG_NAME(obj, debug);
}

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
