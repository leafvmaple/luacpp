#include "lparser.h"
#include "llex.h"
#include "lstring.h"
#include "lparser.h"
#include "lfunc.h"
#include "ltable.h"
#include "lcode.h"

/*
** prototypes for recursive non-terminal functions
*/

// 获取Token是否等于c，是则获取下一个Token并返回true
int LexState::testnext(int c) {
    if (t.token == c) {
        nexttoken();
        return 1;
    }
    return 0;
}

void LexState::checknext(int c) {
    // check(c);
    nexttoken();
}

void LexState::check_match(int what) {
    testnext(what);
}

TString* LexState::str_checkname() {
    TString* ts = t.seminfo.ts;
    nexttoken();
    return ts;
}

static void init_exp(expdesc* e, expkind k, int i) {
    e->k = k;
    e->info = i;
}

void LexState::codestring(expdesc* e, TString* s) {
    init_exp(e, VK, fs->stringK(s));
}

static int singlevaraux(FuncState* fs, TString* n, expdesc* var, int base) {
    if (fs == nullptr) {
        init_exp(var, VGLOBAL, NO_REG);
        return VGLOBAL;
    }
    if (singlevaraux(fs->prev, n, var, 0) == VGLOBAL)
        return VGLOBAL;
}

void LexState::singlevar(expdesc* var) {
    TString* varname = str_checkname();
    if (singlevaraux(fs, varname, var, 1) == VGLOBAL)
        var->info = fs->stringK(varname);
}

static BinOpr getbinopr(int op) {
    switch (op) {
    case '+': return OPR_ADD;
    case '-': return OPR_SUB;
    case '*': return OPR_MUL;
    case '/': return OPR_DIV;
    case '%': return OPR_MOD;
    case '^': return OPR_POW;
    case TK_CONCAT: return OPR_CONCAT;
    case TK_NE: return OPR_NE;
    case TK_EQ: return OPR_EQ;
    case '<': return OPR_LT;
    case TK_LE: return OPR_LE;
    case '>': return OPR_GT;
    case TK_GE: return OPR_GE;
    case TK_AND: return OPR_AND;
    case TK_OR: return OPR_OR;
    default: return OPR_NOBINOPR;
    }
}

void LexState::pushclosure(FuncState* func, expdesc* v) {
    Proto* f = fs->f;

    f->p.emplace_back(func->f);
    init_exp(v, VRELOCABLE, fs->codeABx(OP_CLOSURE, 0, 0));
}

// 新增一层FuncState
void LexState::openfunc(FuncState* fs) {
    fs->prev = this->fs;
    fs->ls = this;
    this->fs = fs;

    L->top++->setvalue(fs->h, "#[Open Function] Const String Table#");
    L->top++->setvalue(fs->f, "#[Open Function] Proto#");
}

void LexState::closefunc() {
    fs->ret(0, 0);  /* final return */
    fs = fs->prev;
    L->top -= 2;  /* remove table and prototype from the stack */
    /* last token read was anchored in defunct function; must reanchor it */
}

// 是否为block结束
static bool block_follow(int token) {
    switch (token)
    {
    case TK_EOS:
    case TK_END:
        return true;
    default:
        return false;
    }
}

void LexState::chunk() {
    ++L->nCcalls;
    while (!block_follow(t.token)) {
        statement();
    }
    --L->nCcalls;
}

// 将文件解析为一个Proto
Proto* luaY_parser(lua_State* L, ZIO* z, const char* name) {
    LexState ls;
    FuncState fs(L);

    ls.setinput(L, z, strtab(L)->newstr(L, name));
    ls.openfunc(&fs);
    ls.nexttoken(); // 读取该文件第一个Token
    ls.chunk();
    ls.closefunc();

    return fs.f;
}

void LexState::body(expdesc* e, int needself, int line) {
    /* body ->  `(' parlist `)' chunk END */
    FuncState fs(L);
    openfunc(&fs);
    nexttoken(); // read '('
    nexttoken(); // read ')'
    chunk();
    check_match(TK_END); // read 'end'
    closefunc();
    pushclosure(&fs, e);
}

int LexState::explist1(expdesc* v) {
    /* explist1 -> expr { `,' expr } */
    int n = 1;
    expr(v);
    while (testnext(',')) {
        n++;
    }

    return n;
}

/* funcstat -> FUNCTION funcname body */
void LexState::funcargs(expdesc* f) {
    int base = 0;
    int nparams = 0;
    expdesc args;

    switch (t.token) {
    case '(': {
        nexttoken();
        if (t.token == ')')  /* arg list is empty? */
            args.k = VVOID;
        else
            explist1(&args);
        check_match(')');
        break;
    }
    default: {
        return;
    }
    }
    base = f->info;
    if (args.k != VVOID)
        fs->exp2nextreg(&args);
    nparams = fs->freereg - (base + 1);
    init_exp(f, VCALL, fs->codeABC(OP_CALL, base, nparams + 1, 2));
}

/* prefixexp -> NAME */
void LexState::prefixexp(expdesc* var) {
    switch (t.token)
    {
    case TK_NAME: {
        singlevar(var);
        break;
    }
    default:
        break;
    }
}

/* primaryexp -> prefixexp { `(' funcargs `)' } */
void LexState::primaryexp(expdesc* v) {
    prefixexp(v);
    while (true) {
        switch (t.token)
        {
        case '(': {
            fs->exp2nextreg(v);
            funcargs(v);
            break;
        }
        default: return;
        }
    }
}

/* simpleexp -> NUMBER | STRING | primaryexp */
void LexState::simpleexp(expdesc* v) {
    switch (t.token) {
    case TK_NUMBER: {
        init_exp(v, VKNUM, 0);
        v->nval = t.seminfo.r;
        break;
    }
    case TK_STRING: {
        codestring(v, t.seminfo.ts);
        break;
    }
    default:
        primaryexp(v);
        return;
    }
    nexttoken();
}

/*
** subexpr -> simpleexp
** where `binop' is any binary operator with a priority higher than `limit'
*/
BinOpr LexState::subexpr(expdesc* v, unsigned int limit) {
    BinOpr op = OPR_NOBINOPR;

    L->nCcalls++;
    simpleexp(v);
    L->nCcalls--;

    return op;
}

void LexState::expr(expdesc* v) {
    subexpr(v, 0);
}

void LexState::assignment(expdesc* lh, int nvars) {
    expdesc e;
    int nexps = 0;
    checknext('=');
    nexps = explist1(&e);
    fs->storevar(lh, &e);
}

int LexState::funcname(expdesc* v) {
    /* funcname -> NAME {field} [`:' NAME] */
    singlevar(v);
    return false;
}

void LexState::funcstat(int line) {
    /* funcstat -> FUNCTION funcname body */
    int needself;
    expdesc v, b;
    nexttoken();  /* skip FUNCTION */
    needself = funcname(&v);
    body(&b, needself, line);
    fs->storevar(&v, &b);
}


void LexState::exprstat() {
    /* exprstat -> func | assignment */
    expdesc desc;
    primaryexp(&desc);
    if (desc.k == VCALL)
        fs->getcode(&desc).c = 1;
    else
        assignment(&desc, 1);
}

void LexState::statement() {
    int line = linenumber;  /* may be needed for error messages */
    switch (t.token)
    {
    case TK_FUNCTION: {
        funcstat(linenumber);  /* stat -> funcstat */
    }
    default: {
        exprstat();
    }
    }
}
