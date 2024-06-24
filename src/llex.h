#pragma once

#include "lobject.h"
#include "lzio.h"

struct FuncState;

union SemInfo {
    lua_Number r   = 0;
    TString*   ts;
};

struct Token {
    int     token    = 0;
    SemInfo seminfo;
};

struct LexState {
    int         current    = 0;
    int         linenumber = 1;
    int         lastline   = 1; /* 最后一个parse的token所在的行 */
    Token       t;              // 当前Token
    FuncState*  fs         = nullptr;
    lua_State*  L          = nullptr;
    ZIO*        z          = nullptr;
    std::string buff;
    TString*    source     = nullptr;

    TString* newstring(const char* str, size_t l);
    void setinput(lua_State* L, ZIO* z, TString* source);
    void nexttoken();
};

void luaX_init(lua_State* L);
