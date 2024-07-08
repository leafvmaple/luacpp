#include <ctype.h>

#include "llex.h"
#include "lstring.h"
#include "lparser.h"
#include "ltable.h"

// 读取下一个字符
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
        TString* ts = strtab(L)->newstr(L, luaX_tokens[i]);
        ts->fix();
        ts->reserved = (RESERVED)(i + FIRST_RESERVED);
    }
}

TString* LexState::newstring(const char* str, size_t l) {
    TString* ts = strtab(L)->newlstr(L, str, l);
    TValue* o = fs->h->setstr(L, ts);
    o->setvalue(true);
    return ts;
}

void LexState::setinput(lua_State* L, ZIO* z, TString* source) {
    this->L = L;
    this->z = z;
    this->source = source;
    next(this);
}

/* LUA_NUMBER */
void LexState::read_numeral(SemInfo* seminfo) {
    do {
        save_and_next(this);
    } while (isdigit(current) || current == '.');
    while (isalnum(current) || current == '_')
        save_and_next(this);
    luaO_str2d(buff.c_str(), &seminfo->r);
}

void LexState::read_string(int del, SemInfo* seminfo) {
    save_and_next(this);
    while (current != del) {
        save_and_next(this);
    }
    save_and_next(this);  /* skip delimiter */
    seminfo->ts = newstring(buff.c_str() + 1, buff.size() - 2);
}

// 解析Token
int LexState::llex(SemInfo* seminfo) {
    buff.clear();
    while (true) {
        switch (current) {
        case '"':
        case '\'': {
            read_string(current, seminfo);
            return TK_STRING;
        }
        case EOZ: {
            return TK_EOS;
        }
        default: {
            if (isspace(current)) {
                next(this);
                continue;
            }
            else if (isdigit(current)) {
                read_numeral(seminfo);
                return TK_NUMBER;
            }
            else if (isalpha(current) || current == '_') {
                do {
                    save_and_next(this);
                } while (isalnum(current) || current == '_');
                TString* ts = newstring(buff.c_str(), buff.size());
                if (ts->reserved)
                    return ts->reserved;
                else {
                    seminfo->ts = ts;
                    return TK_NAME;
                }
            }
            else {
                int c = current;
                next(this);
                return c;  /* single-char tokens (+ - / ...) */
            }
        }
        }
    }
}

void LexState::nexttoken() {
    lastline = linenumber;
    t.token = llex(&t.seminfo);
}