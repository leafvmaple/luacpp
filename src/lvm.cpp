#include "lvm.h"
#include "ldo.h"
#include "ltable.h"
#include "lstate.h"
#include "lstring.h"
#include "luaconf.h"
#include "lopcodes.h"

int luaV_tostring(lua_State* L, TValue* obj) {
    char s[LUAI_MAXNUMBER2STR];
    lua_Number n = obj->value.n;
    lua_number2str(s, n);
    obj->setvalue(strtab(L)->newstr(L, s), s);
    return 1;
}

// ִ��������ϵ�savedpcָ��
void LClosure::execute(lua_State* L, int nexeccalls) {
    const Instruction* pc = L->savedpc;
    auto& k = p->k;

    while (true) {
        const Instruction i = *pc++;
        TValue* ra = &L->base[i.a];

        switch (i.op)
        {
            // Load Const
        case OP_LOADK: {
            *ra = k[i.bc];
            continue;
        }
        case OP_GETGLOBAL: {
            *ra = (*env)[&k[i.bc]];
            continue;
        }
        case OP_RETURN: {
            int b = i.b;
            if (b != 0)
                L->top = ra + b - 1;
            L->savedpc = pc;
            // b = luaD_poscall(L, ra);
            if (--nexeccalls == 0)  /* was previous function running `here'? */
                return;  /* no: return */
        }
        case OP_CALL: {
            // bΪ������+��������
            int b = i.b;
            if (b != 0)
                L->top = ra + b;
            L->savedpc = pc;
            Closure* c = reinterpret_cast<Closure*>(ra);
            c->precall(L, ra, 0);
            continue;
        }
        case OP_SETGLOBAL: {
            (*env)[&k[i.bc]] = *ra;
            continue;
        }
        default:
            break;
        }
    }
}
