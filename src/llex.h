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
};

void luaX_init(lua_State* L);
TString* luaX_newstring(LexState* ls, const char* str, size_t l);
void luaX_setinput(lua_State* L, LexState* ls, ZIO* z, TString* source);
// 记录当前token位置，并获取下一个token
void luaX_next(LexState* ls);