#include "lvm.h"
#include "ldo.h"
#include "ltable.h"
#include "lstate.h"
#include "lstring.h"
#include "luaconf.h"
#include "lopcodes.h"

int TValue::tostring(lua_State* L) {
    if (!isnumber())
        return 0;
    {
        char s[LUAI_MAXNUMBER2STR];
        lua_number2str(s, n);
        setvalue(strtab(L)->newstr(L, s), s);
        return 1;
    }
}

// 执行虚拟机上的savedpc指令
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
                L->stack.settop(ra + b - 1);
            L->savedpc = pc;
            // b = luaD_poscall(L, ra);
            if (--nexeccalls == 0)  /* was previous function running `here'? */
                return;  /* no: return */
        }
        case OP_CALL: {
            // b为函数名+参数个数
            int b = i.b;
            int nresults = i.c - 1;
            if (b != 0)
                L->stack.settop(ra + b);
            L->savedpc = pc;
            Closure* c = ra->cl;
            c->precall(L, ra, nresults);
            continue;
        }
        case OP_SETGLOBAL: {
            (*env)[&k[i.bc]] = *ra;
            continue;
        }
        case OP_NEWTABLE: {
            ra->setvalue(new Table(L, i.b, i.c), "#New Table#");
            continue;
        }
        default:
            break;
        }
    }
}
