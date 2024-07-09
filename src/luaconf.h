#pragma once

#define LUA_PROMPT          "> "
#define LUA_PROMPT2         ">> "

#define LUA_NUMBER_FMT      "%.14g"

#define LUAI_MAXNUMBER2STR  32 /* 16 digits, sign, point, and \0 */
#define LUA_MAXINPUT        512

#define LUAI_MEM	ptrdiff_t

inline int lua_readline(char* buff, const char* prmt) {
    fputs(prmt, stdout);
    fflush(stdout);
    return fgets(buff, LUA_MAXINPUT, stdin) != nullptr;
}

inline void lua_number2str(char* s, lua_Number n) {
    sprintf(s, LUA_NUMBER_FMT, n);
}

inline double lua_str2number(const char* s, char** p) {
    return strtod(s, p);
}