#include "lcode.h"
#include "lparser.h"
#include "lobject.h"
#include "ltable.h"
#include "lfunc.h"
#include "llex.h"

// 给FuncState记录一个新增变量
static int addk(FuncState* fs, TValue* k, TValue* v) {
    TValue* idx = luaH_set(fs->L, fs->h, k);
    
    int nk = (int)fs->f->k.size();
    setnvalue(idx, nk);
    fs->f->k.push_back(*v);
    return nk;
}

static int luaK_code(FuncState* fs, Instruction i, int line) {
    Proto* f = fs->f;
    f->code.push_back(i);
    f->lineinfo.push_back(line);
    return fs->pc++;
}

static void discharge2reg(FuncState* fs, expdesc* e, int reg) {
    luaK_dischargevars(fs, e);
    switch (e->k) {
    case VK: {
        luaK_codeABx(fs, OP_LOADK, reg, e->u.s.info);
        break;
    }
    case VKNUM: {
        luaK_codeABx(fs, OP_LOADK, reg, luaK_numberK(fs, e->u.nval));
        break;
    }
    case VRELOCABLE: {
        // 可重新分配，直接把新寄存器位置设到指令A参数即可（指令会通过A参数从指定寄存器取值）
        Instruction* pc = &getcode(fs, e);
        SETARG_A(*pc, reg);
        break;
    }
    case VNONRELOC: {
        if (reg != e->u.s.info)
            luaK_codeABC(fs, OP_MOVE, reg, e->u.s.info, 0);
        break;
    }
    default: {
        return;  /* nothing to do... */
    }
    }
    e->u.s.info = reg;
    e->k = VNONRELOC;
}


static void exp2reg(FuncState* fs, expdesc* e, int reg) {
    discharge2reg(fs, e, reg);
    e->u.s.info = reg;
    e->k = VNONRELOC;
}


int luaK_codeABx(FuncState* fs, OpCode o, int A, unsigned int Bx) {
    return luaK_code(fs, CREATE_ABx(o, A, Bx), fs->ls->lastline);
}

int luaK_codeABC(FuncState* fs, OpCode o, int A, int B, int C) {
    return luaK_code(fs, CREATE_ABC(o, A, B, C), fs->ls->lastline);
}

int luaK_exp2anyreg(FuncState* fs, expdesc* e) {
    luaK_dischargevars(fs, e);
    luaK_exp2nextreg(fs, e);
    return e->u.s.info;
}

void luaK_reserveregs(FuncState* fs, int n) {
    fs->freereg += n;
}

void luaK_dischargevars(FuncState* fs, expdesc* e) {
    switch (e->k) {
    case VGLOBAL: {
        // e->u.s.info在之前被赋值为标识符常量序号
        // code = fs->h[e->u.s.info]
        // A参数暂时设0，等待重分配的时候再赋予正确的值
        // 处理完后，e->u.s.info指向指令位置
        e->u.s.info = luaK_codeABx(fs, OP_GETGLOBAL, 0, e->u.s.info);
        e->k = VRELOCABLE;
        break;
    }
    }
}

void luaK_exp2nextreg(FuncState* fs, expdesc* e) {
    luaK_dischargevars(fs, e);
    luaK_reserveregs(fs, 1);
    exp2reg(fs, e, fs->freereg - 1);
}

void luaK_storevar(FuncState* fs, expdesc* var, expdesc* ex) {
    switch (var->k)
    {
    case VGLOBAL: {
        int e = luaK_exp2anyreg(fs, ex);
        luaK_codeABx(fs, OP_SETGLOBAL, e, var->u.s.info);
        break;
    }
    default:
        break;
    }
}

void luaK_ret(FuncState* fs, int first, int nret) {
    luaK_codeABC(fs, OP_RETURN, first, nret + 1, 0);
}

int luaK_stringK(FuncState* fs, TString* s) {
    TValue o;
    setsvalue(&o, s, s->s.c_str());
    return addk(fs, &o, &o);
}

int luaK_numberK(FuncState* fs, lua_Number r) {
    TValue o;
    setnvalue(&o, r);
    return addk(fs, &o, &o);
}
