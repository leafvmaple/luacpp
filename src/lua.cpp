#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luaconf.h"

static void print_version(void) {
    fprintf(stderr, "%s\n", LUA_VERSION "  " LUA_COPYRIGHT);
    fflush(stderr);
}

const char* get_prompt(lua_State* L, int firstline) {
    return firstline ? LUA_PROMPT : LUA_PROMPT2;
}

int pushline(lua_State* L, int firstline) {
    char buff[LUA_MAXINPUT];
    size_t l = 0;

    const char* prmt = get_prompt(L, firstline);
    if (!lua_readline(buff, prmt))
        return 0;
    l = strlen(buff);
    if (l > 0 && buff[l - 1] == '\n')
        buff[l - 1] = '\0';
    lua_pushstring(L, buff);
    return 1;
}

int loadline(lua_State* L) {
    int status = 0;
    lua_settop(L, 0);
    if (!pushline(L, true))
        return -1;

    return status;
}

static int docall(lua_State* L, int narg, int clear) {
    lua_call(L, narg, 0);
    return true;
}

void dotty(lua_State* L) {
    int status = 0;
    while ((status = loadline(L)) != -1) {
        size_t len = 0;
        const char* s = lua_tolstring(L, -1, &len);
        status = luaL_loadbuffer(L, s, len, "=stdin");
        status = docall(L, 0, 0);
    }
}

struct Smain {
    int argc    = 0;
    char** argv = nullptr;
    int status  = 0;
};

static int pmain(lua_State* L) {
    Smain* s = (struct Smain*)lua_touserdata(L, 1);
    luaL_openlibs(L);
    print_version();
    dotty(L);
    return 0;
}

int main(int argc, char** argv) {
    int ret = 0;
    struct Smain s;

    s.argc = argc;
    s.argv = argv;

    lua_State* L = luaL_newstate();
    ret = lua_cpcall(L, &pmain, &s);
}
