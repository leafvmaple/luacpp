#include "lcode.h"
#include "lparser.h"
#include "lobject.h"
#include "ltable.h"
#include "lfunc.h"
#include "llex.h"

Instruction& FuncState::getcode(expdesc* e) {
    return f->code[e->info];
}

// ��FuncState��¼һ����������
// h[k] = idx
// f->k[idx] = v
int FuncState::addk(TValue* k, TValue* v) {
    size_t nk = f->k.size();
    (*h)[k] = (lua_Number)nk;
    f->k.emplace_back(*v);
    return nk;
}

int FuncState::code(Instruction i, int line) {
    int count = f->code.size();
    f->code.emplace_back(i);
    f->line.emplace_back(line);
    return count;
}

void FuncState::discharge2reg(expdesc* e, int reg) {
    dischargevars(e);
    switch (e->k) {
    case VK: {
        codeABx(OP_LOADK, reg, e->info);
        break;
    }
    case VKNUM: {
        codeABx(OP_LOADK, reg, numberK(e->nval));
        break;
    }
    case VRELOCABLE: {
        // �����·��䣬ֱ�Ӱ��¼Ĵ���λ���赽ָ��A�������ɣ�ָ���ͨ��A������ָ���Ĵ���ȡֵ��
        getcode(e).a = reg;
        break;
    }
    case VNONRELOC: {
        if (reg != e->info)
            codeABC(OP_MOVE, reg, e->info, 0);
        break;
    }
    default: {
        return;  /* nothing to do... */
    }
    }
    e->info = reg;
    e->k = VNONRELOC;
}

void FuncState::exp2reg(expdesc* e, int reg) {
    discharge2reg(e, reg);
    e->info = reg;
    e->k = VNONRELOC;
}

int FuncState::codeABx(OpCode o, int A, unsigned int Bx) {
    return code(CREATE_ABx(o, A, Bx), ls->lastline);
}

int FuncState::codeABC(OpCode o, int A, int B, int C) {
    return code(CREATE_ABC(o, A, B, C), ls->lastline);
}

int FuncState::exp2anyreg(expdesc* e) {
    dischargevars(e);
    if (e->k == VNONRELOC)
        return e->info;
    exp2nextreg(e);
    return e->info;
}

void FuncState::reserveregs(int n) {
    freereg += n;
}

void FuncState::dischargevars(expdesc* e) {
    switch (e->k) {
    case VGLOBAL: {
        // e->info��֮ǰ����ֵΪ��ʶ���������
        // code = fs->h[e->info]
        // A������ʱ��0���ȴ��ط����ʱ���ٸ�����ȷ��ֵ
        // �������e->infoָ��ָ��λ��
        e->info = codeABx(OP_GETGLOBAL, 0, e->info);
        e->k = VRELOCABLE;
        break;
    }
    }
}

// ����һ���¼Ĵ���������exp
void FuncState::exp2nextreg(expdesc* e) {
    dischargevars(e);
    reserveregs(1);
    exp2reg(e, freereg - 1);
}

void FuncState::storevar(expdesc* var, expdesc* ex) {
    switch (var->k)
    {
    case VGLOBAL: {
        int e = exp2anyreg(ex);
        codeABx(OP_SETGLOBAL, e, var->info);
        break;
    }
    default:
        break;
    }
}

void FuncState::ret(int first, int nret) {
    codeABC(OP_RETURN, first, nret + 1, 0);
}

// ���볣����
int FuncState::stringK(TString* s) {
    TValue o(s, s->s.c_str());
    return addk(&o, &o);
}

int FuncState::numberK(lua_Number r) {
    TValue o(r);
    return addk(&o, &o);
}
