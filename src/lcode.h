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
