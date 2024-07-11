#pragma once

#include "lobject.h"
#include "lzio.h"

struct LexState;
struct expdesc;

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
    int        freereg = 0;  /* first free register */

    FuncState(lua_State* L) {
        f = new (L) Proto(L);
        h = new (L) Table(L, 0, 0);
    }

    Instruction& getcode(expdesc* e);
    void discharge2reg(expdesc* e, int reg);
    void exp2reg(expdesc* e, int reg);

    int codeABx(OpCode o, int A, unsigned int Bx);
    int codeABC(OpCode o, int A, int B, int C);
    int exp2anyreg(expdesc* e);
    void reserveregs(int n);
    void dischargevars(expdesc* e);
    void exp2nextreg(expdesc* e);
    void storevar(expdesc* var, expdesc* ex);
    void ret(int first, int nret);
    int stringK(TString* s);
    int numberK(lua_Number r);

    int code(Instruction i, int line);

private:
    int addk(TValue* k, TValue* v);
};

struct expdesc {
    expkind k;
    union {
        struct {
            // VGLOBAL: 常量表索引
            // VRELOCABLE: 指令PC
            int info;
            int aux;
        };
        lua_Number nval;  // 表达式为数字
    };
};

Proto* luaY_parser(lua_State* L, ZIO* z, const char* name);