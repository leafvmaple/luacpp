#include "lgc.h"
#include "lstate.h"

static lua_GCList::iterator sweeplist(lua_State* L, lua_GCList::iterator it, lua_GCList& list, lu_mem count) {
    global_State* g = G(L);

    for (int i = 0; i < count; i++) {
        if (!(*it)->marked.isdead(g->currentwhite) || (*it)->marked[FIXEDBIT]) {
            (*it)->marked.maskmarks(g->currentwhite);
        }
        else {
            delete *it;
            it = list.erase(it);
        }
    }

    return it;
}

static l_mem singlestep(lua_State* L) {
    global_State* g = G(L);
    switch (g->gcstate) {
    case GCSsweepstring:
        auto& list  = g->sweepstrgc->second;
        sweeplist(L, list.begin(), list, list.size());
        if (++g->sweepstrgc == strtab(L)->hash.end())
            g->gcstate = GCSsweep;
        break;
    }
    return 0;
}

// 将可GC对象挂到GC列表中，同时标记为白色
void luaC_link(lua_State* L, GCheader* o, TVALUE_TYPE tt) {
    global_State* g = G(L);

    g->rootgc.emplace_front(o);
    o->tt = tt;
    o->marked.white(g->currentwhite);
}

void lua_Marked::white(const lua_Marked& in, bool reset) {
    if (reset)
        bit.reset();
    bit[WHITE0BIT] = in[WHITE0BIT];
    bit[WHITE1BIT] = in[WHITE1BIT];
}

int lua_Marked::isdead(const lua_Marked& gm) const {
    return (bit[WHITE0BIT] ^ gm[WHITE0BIT]) & (bit[WHITE1BIT] ^ gm[WHITE1BIT]);
}

void lua_Marked::maskmarks(const lua_Marked& gm) {
    bit[BLACKBIT] = 0;
    white(gm, false);
}

void luaC_fullgc(lua_State* L) {
    global_State* g = G(L);
    if (g->gcstate <= GCSpropagate) {
        g->sweepstrgc = strtab(L)->hash.begin();
        g->sweepgc = g->rootgc.begin();
        g->gcstate = GCSsweepstring;
    }

    while (g->gcstate != GCSfinalize) {
        singlestep(L);
    }
}