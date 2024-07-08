#pragma once

#include "lobject.h"
#include "lzio.h"
#include "lcode.h"

struct FuncState;

union SemInfo {
    lua_Number r = 0;
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

    int testnext(int c);
    void checknext(int c);
    void check_match(int what);
    TString* str_checkname();

    void codestring(expdesc* e, TString* s);
    void singlevar(expdesc* var);

    void pushclosure(FuncState* func, expdesc* v);
    void openfunc(FuncState* fs);
    void closefunc();

    void chunk();
    void body(expdesc* e, int needself, int line);
    int explist1(expdesc* v);
    void funcargs(expdesc* f);

    void prefixexp(expdesc* var);
    void primaryexp(expdesc* v);
    void simpleexp(expdesc* v);
    BinOpr subexpr(expdesc* v, unsigned int limit);
    void expr(expdesc* v);

    void assignment(expdesc* lh, int nvars);
    int funcname(expdesc* v);
    void funcstat(int line);
    void exprstat();
    void statement();

    void read_numeral(SemInfo* seminfo);
    void read_string(int del, SemInfo* seminfo);
    int llex(SemInfo* seminfo);
    void nexttoken();
};

void luaX_init(lua_State* L);
