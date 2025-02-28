#include "lualib.h"
#include "lauxlib.h"

namespace Base {
    static int tostring(lua_State* L) {
        switch (lua_type(L, 1)) {
        case LUA_TNUMBER:
            lua_pushstring(L, lua_tostring(L, 1));
            break;
        case LUA_TSTRING:
            lua_pushvalue(L, 1);
            break;
        }
        return 1;
    }
    /*
    ** If your system does not support `stdout', you can just remove this function.
    ** If you need, you can define your own `print' function, following this
    ** model but changing `fputs' to put the strings at a proper place
    ** (a console window or a log file, for instance).
    */
    static int print(lua_State* L) {
        int n = lua_gettop(L);  /* number of arguments */
        for (int i = 1; i <= n; i++) {
            const char* s = lua_tostring(L, -1);  /* get result */
            if (i > 1) fputs("\t", stdout);
            fputs(s, stdout);
            lua_pop(L, 1);  /* pop result */
        }
        fputs("\n", stdout);
        return 0;
    }
    static int collectgarbage(lua_State* L) {
        static const char* const opts[] = { "collect", nullptr };
        static const int optsnum[] = { LUA_GCCOLLECT };
        int o = luaL_checkoption(L, 1, "collect", opts);
        int res = lua_gc(L, optsnum[o], 0);

        return 0;
    }
    static int setmetatable(lua_State* L) {
        lua_settop(L, 2);
        lua_setmetatable(L, 1);
        return 1;
    }
}

static const luaL_Reg base_funcs[] = {
    {"collectgarbage", Base::collectgarbage},
    {"print",          Base::print},
    {"tostring",       Base::tostring},
    {"setmetatable",   Base::setmetatable},
    {nullptr, nullptr}
};

void base_open(lua_State* L) {
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    lua_setglobal(L, "_G");
    luaL_register(L, "_G", base_funcs);

    lua_pushstring(L, LUA_VERSION);
    lua_setglobal(L, "_VERSION");
}

int luaopen_base(lua_State* L) {
    base_open(L);
    return 1;
}