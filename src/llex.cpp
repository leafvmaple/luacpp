#include <ctype.h>

#include "llex.h"
#include "lstring.h"
#include "lparser.h"
#include "ltable.h"

void next(LexState* ls) {
    ls->current = zgetc(ls->z);
}

void save(LexState* ls, int c) {
    ls->buff.push_back(c);
}

void save_and_next(LexState* ls) {
    save(ls, ls->current);
    next(ls);
}

const char* const luaX_tokens[] = {
    "and",
    "break",
    "do",
    "else",
    "elseif",
    "end",
    "false",
    "for",
    "function",
    "if",
    "in",
    "local",
    "nil",
    "not",
    "or",
    "repeat",
    "return",
    "then",
    "true",
    "until",
    "while",
    "..", "...", "==", ">=", "<=", "~=",
    "<number>", "<name>", "<string>", "<eof>",
    NULL
};

// 初始化语法关键字
void luaX_init(lua_State* L) {
    for (int i = 0; i < NUM_RESERVED; i++) {
        TString* ts = luaS_new(L, luaX_tokens[i]);
        luaS_fix(ts);
        ts->reserved = (RESERVED)(i + FIRST_RESERVED);
    }
}

TString* luaX_newstring(LexState* ls, const char* str, size_t l) {
    lua_State* L = ls->L;
    TString* ts = luaS_newlstr(L, str, l);
    TValue* o = luaH_setstr(L, ls->fs->h, ts);
    setbvalue(o, true);
    return ts;
}

void luaX_setinput(lua_State* L, LexState* ls, ZIO* z, TString* source) {
    ls->L = L;
    ls->z = z;
    ls->source = source;
    next(ls);
}

/* LUA_NUMBER */
static void read_numeral(LexState* ls, SemInfo* seminfo) {
    do {
        save_and_next(ls);
    } while (isdigit(ls->current) || ls->current == '.');
    while (isalnum(ls->current) || ls->current == '_')
        save_and_next(ls);
    luaO_str2d(ls->buff.c_str(), &seminfo->r);
}

static void read_string(LexState* ls, int del, SemInfo* seminfo) {
    save_and_next(ls);
    while (ls->current != del) {
        save_and_next(ls);
    }
    save_and_next(ls);  /* skip delimiter */
    seminfo->ts = luaX_newstring(ls, ls->buff.c_str() + 1, ls->buff.size() - 2);
}

static int llex(LexState* ls, SemInfo* seminfo) {
    ls->buff.clear();
    while (true) {
        switch (ls->current) {
        case '"':
        case '\'': {
            read_string(ls, ls->current, seminfo);
            return TK_STRING;
        }
        case EOZ: {
            return TK_EOS;
        }
        default: {
            if (isspace(ls->current)) {
                next(ls);
                continue;
            }
            else if (isdigit(ls->current)) {
                read_numeral(ls, seminfo);
                return TK_NUMBER;
            }
            else if (isalpha(ls->current) || ls->current == '_') {
                TString* ts = nullptr;
                do {
                    save_and_next(ls);
                } while (isalnum(ls->current) || ls->current == '_');
                ts = luaX_newstring(ls, ls->buff.c_str(), ls->buff.size());
                if (ts->reserved)
                    return ts->reserved;
                else {
                    seminfo->ts = ts;
                    return TK_NAME;
                }
            }
            else {
                int c = ls->current;
                next(ls);
                return c;  /* single-char tokens (+ - / ...) */
            }
        }
        }
    }
}

void luaX_next(LexState* ls){
    ls->lastline = ls->linenumber;
    ls->t.token = llex(ls, &ls->t.seminfo);
}
