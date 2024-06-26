#include "ltm.h"
#include "lstring.h"
#include "lstate.h"

// ��ʼ��metatable�ؼ���
void luaT_init(lua_State* L) {
    static const char* const luaT_eventname[] = {  /* ORDER TM */
      "__index",
      "__newindex",
      "__gc",
      "__mode",
      "__eq",
      "__add",
      "__sub",
      "__mul",
      "__div",
      "__mod",
      "__pow",
      "__unm",
      "__len",
      "__lt",
      "__le",
      "__concat",
      "__call"
    };

    for (int i = 0; i < TM_N; i++) {
        G(L)->tmname[i] = strtab(L)->newstr(L, luaT_eventname[i]);
        G(L)->tmname[i]->fix();
    }
}