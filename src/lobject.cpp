#include "lobject.h"
#include "luaconf.h"

void setnilvalue(TValue* obj _IMPL) {
    obj->tt = LUA_TNIL;
    SET_DEBUG_NAME(obj, debug);
}

void setnvalue(TValue* obj, const lua_Number n _IMPL) {
    obj->value.n = n;
    obj->tt = LUA_TNUMBER;
    SET_DEBUG_NAME(obj, debug);
}

void setpvalue(TValue* obj, void* p _IMPL) {
    obj->value.p = p;
    obj->tt = LUA_TLIGHTUSERDATA;
    SET_DEBUG_NAME(obj, debug);
}

void setbvalue(TValue* obj, const bool b _IMPL) {
    obj->value.b = b;
    obj->tt = LUA_TBOOLEAN;
    SET_DEBUG_NAME(obj, debug);
}

void setsvalue(TValue* obj, const TString* s _IMPL) {
    obj->value.gc = const_cast<TString*>(s);
    obj->tt = LUA_TSTRING;
    SET_DEBUG_NAME(obj, debug);
}

void sethvalue(TValue* obj, const Table* h _IMPL) {
    obj->value.gc = const_cast<Table*>(h);
    obj->tt = LUA_TTABLE;
    SET_DEBUG_NAME(obj, debug);
}

void setclvalue(TValue* obj, const Closure* cl _IMPL) {
    obj->value.gc = const_cast<Closure*>(cl);
    obj->tt = LUA_TFUNCTION;
    SET_DEBUG_NAME(obj, debug);
}

void setptvalue(TValue* obj, const Proto* pt _IMPL) {
    obj->value.gc = const_cast<Proto*>(pt);
    obj->tt = LUA_TPROTO;
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
