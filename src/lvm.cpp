#include "lvm.h"
#include "ldo.h"
#include "ltable.h"
#include "lstate.h"
#include "lstring.h"
#include "luaconf.h"
#include "lopcodes.h"

inline TValue* RA(TValue* base, Instruction i) {
    return base + GETARG_A(i);
}
inline TValue* RB(TValue* base, Instruction i) {
    return base + GETARG_B(i);
}
inline TValue* RC(TValue* base, Instruction i) {
    return base + GETARG_C(i);
}
inline TValue* KBx(TValue* base, Instruction i) {
    return base + GETARG_Bx(i);
}


int luaV_tostring(lua_State* L, TValue* obj) {
    char s[LUAI_MAXNUMBER2STR];
    lua_Number n = obj->value.n;
    lua_number2str(s, n);
    obj->setvalue(strtab(L)->newstr(L, s), s);
    return 1;
}

void luaV_gettable(lua_State* L, const TValue* t, TValue* key, TValue* val) {
    Table* h = (Table*)t->value.gc;
    const TValue* res = h->get(key);
    *val = *res;
}

void luaV_settable(lua_State* L, const TValue* t, TValue* key, TValue* val) {
    TValue* oldval = ((Table*)t->value.gc)->set(L, key);
    *oldval = *val;
}

void luaV_execute(lua_State* L, int nexeccalls) {
    const Instruction* pc = nullptr;
    LClosure* cl = nullptr;
    TValue* base = nullptr;
    TValue* k = nullptr;

    pc = L->savedpc;
    cl = static_cast<LClosure*>(L->ci->func->value.gc);
    base = L->base;
    k = &cl->p->k.front();

    while (true) {
        const Instruction i = *pc++;
        TValue* ra = RA(base, i);

        switch (GET_OPCODE(i))
        {
        // Load Const
        case OP_LOADK: {
            *ra = *KBx(k, i);
            continue;
        }
        case OP_GETGLOBAL: { // 不使用ra
            TValue g(cl->env);
            luaV_gettable(L, &g, KBx(k, i), ra);
            continue;
        }
        case OP_RETURN: {
            int b = GETARG_B(i);
            if (b != 0) L->top = ra + b - 1;
            L->savedpc = pc;
            // b = luaD_poscall(L, ra);
            if (--nexeccalls == 0)  /* was previous function running `here'? */
                return;  /* no: return */
        }
        case OP_CALL: {
            // b为函数名+参数个数
            int b = GETARG_B(i);
            if (b != 0) L->top = ra + b;
            L->savedpc = pc;
            Closure* c = static_cast<Closure*>(ra->value.gc);
            c->precall(L, ra, 0);
            continue;
        }
        case OP_SETGLOBAL: {
            TValue g(cl->env);
            luaV_settable(L, &g, KBx(k, i), ra);
            continue;
        }
        default:
            break;
        }
    }
}
