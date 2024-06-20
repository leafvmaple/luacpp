#pragma once

#include "lobject.h"
#include "lzio.h"

struct LexState;

enum expkind {
    VVOID,	/* no value */
    VNIL,
    VTRUE,
    VFALSE,
    VK,		/* info = index of constant in `k' */
    VKNUM,	/* nval = numerical value */
    VLOCAL,	/* info = local register */
    VUPVAL,       /* info = index of upvalue in `upvalues' */
    VGLOBAL,	/* info = index of table; aux = index of global name in `k' */
    VINDEXED,	/* info = table register; aux = index register (or `k') */
    VJMP,		/* info = instruction pc */
    VRELOCABLE,	/* info = instruction pc */
    VNONRELOC,	/* info = result register */
    VCALL,	/* info = instruction pc */
    VVARARG	/* info = instruction pc */
};

struct FuncState {
	Proto*     f       = nullptr;
	Table*     h       = nullptr;  /* {常量标识符：序号} */
	FuncState* prev    = nullptr; // 上层嵌套函数
    lua_State* L       = nullptr;
	LexState*  ls      = nullptr;
    int        pc      = 0;
    int        freereg = 0;  /* first free register */
};

struct expdesc {
    expkind k;
    union {
        struct { int info, aux; } s;
        lua_Number nval;  // 表达式为数字
    } u;
};

Proto* luaY_parser(lua_State* L, ZIO* z, const char* name);