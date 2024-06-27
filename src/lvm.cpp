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

void luaV_gettable(lua_State* L, const TValue* t, TValue* key, TValue* val) {
    Table* h = (Table*)t->value.gc;
    const TValue* res = h->get(key);
    *val = *res;
}

void luaV_settable(lua_State* L, const TValue* t, TValue* key, TValue* val) {
    TValue* oldval = ((Table*)t->value.gc)->set(L, key);
    *oldval = *val;
}

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
            TValue g(env);
            luaV_gettable(L, &g, &k[i.bc], ra);
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
            // b为函数名+参数个数
            int b = i.b;
            if (b != 0)
                L->top = ra + b;
            L->savedpc = pc;
            Closure* c = static_cast<Closure*>(ra->value.gc);
            c->precall(L, ra, 0);
            continue;
        }
        case OP_SETGLOBAL: {
            TValue g(env);
            luaV_settable(L, &g, &k[i.bc], ra);
            continue;
        }
        default:
            break;
        }
    }
}
