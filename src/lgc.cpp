#include "lgc.h"
#include "lstate.h"

constexpr int GCSWEEPMAX = 40;

static lua_GCList::iterator sweeplist(lua_State* L, lua_GCList::iterator it, lua_GCList& list, lu_mem count) {
    global_State* g = G(L);

    for (int i = 0; it != list.end() && i < count; i++) {
        if (!(*it)->marked.isdead(g) || (*it)->marked[FIXEDBIT]) {
            (*it++)->marked.maskmarks(g);
        }
        else {
            delete (*it);
            it = list.erase(it);
        }
    }

    return it;
}

static void markroot(lua_State* L) {
    global_State* g = G(L);

    g->gray.clear();

    g->mainthread->trymark(g);
    gt(g->mainthread)->markvalue(g);
    registry(L)->markvalue(g);

    g->gcstate = GCSpropagate;
}

// mark a object on gray list
static l_mem propagatemark(global_State* g) {
    GCheader* o = g->gray.front();
    g->gray.pop_front();

    o->markblack(g);
    o->traverse(g);
    return 0;
}

static size_t propagateall(lua_GCList& l, global_State* g) {
	for (auto& o : l) {
		o->markblack(g);
		o->traverse(g);
	}
	l.clear();
	return 0;
}

static void atomic(lua_State* L) {
    global_State* g = G(L);

    L->trymark(g);
    propagateall(g->weak, g);

    g->currentwhite.tootherwhite();
    g->sweepstrgc = strtab(L)->hash.begin();
    g->sweepgc = g->rootgc.begin();
}

static l_mem singlestep(lua_State* L) {
    global_State* g = G(L);
    switch (g->gcstate) {
    case GCSpropagate: {
        if (!g->gray.empty())
            return propagatemark(g);
        else
            atomic(L);  /* finish mark phase */
        g->gcstate = GCSsweepstring;
        break;
    }
    case GCSsweepstring: {
        auto& list = g->sweepstrgc->second;
        sweeplist(L, list.begin(), list, list.size());
        if (++g->sweepstrgc == strtab(L)->hash.end())
            g->gcstate = GCSsweep;
        break;
    }
    case GCSsweep: {
        g->sweepgc = sweeplist(L, g->sweepgc, g->rootgc, GCSWEEPMAX);
        if (g->sweepgc == g->rootgc.end())  /* nothing more to sweep? */
            g->gcstate = GCSfinalize;  /* end sweep phase */
        break;
    }
    case GCSfinalize: {
        g->gcstate = GCSpause;  /* end collection */
        break;
    }
    default:
        break;
    }
    return 0;
}

void GCheader::markobject(global_State* g) {
    marked.togray();
    g->gray.emplace_front(this);
}

int GCheader::traverse(global_State* g) {
    g->gray.pop_front();
    return 0;
}

void GCheader::link(lua_State* _L) {
    L = _L;
    global_State* g = G(L);

    g->rootgc.emplace_front(this);
    marked.towhite(g);
}

void GCheader::trymark(global_State* g) {
    if (marked.iswhite())
        markobject(g);
}

void GCheader::markblack(global_State* g) {
    marked.toblack();
}

void TValue::markvalue(global_State* g) {
    if (tt >= LUA_TSTRING) {
        gc->trymark(g);
    }
}

void lua_Marked::towhite(global_State* g, bool reset) {
    if (reset)
        bit.reset();
    bit[WHITE0BIT] = g->currentwhite[WHITE0BIT];
    bit[WHITE1BIT] = g->currentwhite[WHITE1BIT];
}

void lua_Marked::togray() {
    bit[WHITE0BIT] = 0;
    bit[WHITE1BIT] = 0;
    bit[BLACKBIT] = 0;
}

void lua_Marked::toblack() {
    bit[BLACKBIT] = 1;
}

void lua_Marked::tootherwhite() {
    bit[WHITE0BIT].flip();
    bit[WHITE1BIT].flip();
}

bool lua_Marked::iswhite() const {
    return bit[WHITE0BIT] || bit[WHITE1BIT];
}

bool lua_Marked::isblack() const {
    return bit[BLACKBIT];
}

bool lua_Marked::isgray() const {
    return !iswhite() && !isblack();
}

int lua_Marked::isdead(global_State* g) const {
    return (bit[WHITE0BIT] ^ g->currentwhite[WHITE0BIT]) & (bit[WHITE1BIT] ^ g->currentwhite[WHITE1BIT]);
}

void lua_Marked::maskmarks(global_State* g) {
    bit[BLACKBIT] = 0;
    towhite(g, false);
}

void luaC_checkGC(lua_State* L) {
    global_State* g = G(L);
    if (g->totalbytes >= g->GCthreshold) {
        luaC_fullgc(L);
    }
}

void luaC_fullgc(lua_State* L) {
    global_State* g = G(L);
    if (g->gcstate <= GCSpropagate) {
        g->sweepstrgc = strtab(L)->hash.begin();
        g->sweepgc = g->rootgc.begin();
        g->gcstate = GCSsweepstring;
    }

    // sweepstrgc sweepgc
    while (g->gcstate != GCSfinalize)
        singlestep(L);

    markroot(L);

    while (g->gcstate != GCSpause)
        singlestep(L);
}