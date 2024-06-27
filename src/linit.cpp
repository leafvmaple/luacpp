#include "lualib.h"
#include "lauxlib.h"

const luaL_Reg lualibs[] = {
    { ""     , luaopen_base },
    { nullptr, nullptr      }
};

void luaL_openlibs(lua_State* L) {
    const luaL_Reg* lib = lualibs;
    for (; lib->func; lib++) {
        lua_pushcfunction(L, lib->func, "#[Library] Function#");
        lua_pushstring(L, lib->name, "#[Library] Key#");
        lua_call(L, 1, 0);
    }
}