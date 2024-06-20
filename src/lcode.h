#pragma once

#include "lopcodes.h"
#include "lparser.h"

struct FuncState;

enum BinOpr {
	OPR_ADD,
	OPR_SUB,
	OPR_MUL,
	OPR_DIV,
	OPR_MOD,
	OPR_POW,
	OPR_CONCAT,
	OPR_NE,
	OPR_EQ,
	OPR_LT,
	OPR_LE,
	OPR_GT,
	OPR_GE,
	OPR_AND,
	OPR_OR,
	OPR_NOBINOPR
};

inline Instruction& getcode(FuncState* fs, expdesc* e) {
	return fs->f->code[e->u.s.info];
}

int luaK_codeABx(FuncState* fs, OpCode o, int A, unsigned int Bx);
int luaK_codeABC(FuncState* fs, OpCode o, int A, int B, int C);
int luaK_exp2anyreg(FuncState* fs, expdesc* e);
void luaK_reserveregs(FuncState* fs, int n);

/* 重定位全局变量和upvalue */
/* VGLOBAL:字符串序号 -> VRELOCABLE:指令序号 */
void luaK_dischargevars(FuncState* fs, expdesc* e);

void luaK_exp2nextreg(FuncState* fs, expdesc* e);

void luaK_storevar(FuncState* fs, expdesc* var, expdesc* e);
void luaK_ret(FuncState* fs, int first, int nret);
int luaK_stringK(FuncState* fs, TString* s);
int luaK_numberK(FuncState* fs, lua_Number r);