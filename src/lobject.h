#pragma once
#pragma warning(disable : 4200)

#include <list>
#include <vector>
#include <unordered_map>
#include <bitset>

#include "lua.h"
#include "llimits.h"

/*
** Layout for bit use in `marked' field:
** bit 0 - object is white (type 0)
** bit 1 - object is white (type 1)
** bit 2 - object is black
** bit 3 - for userdata: has been finalized
** bit 3 - for tables: has weak keys
** bit 4 - for tables: has weak values
** bit 5 - object is fixed (should not be collected)
** bit 6 - object is "super" fixed (only the main thread)
*/

enum MARKED_TYPE {
    WHITE0BIT,
    WHITE1BIT,
    BLACKBIT,
    FINALIZEDBIT,
    KEYWEAKBIT = FINALIZEDBIT,
    VALUEWEAKBIT,
    FIXEDBIT, // 不会GC
    SFIXEDBIT,

    MARKED_COUNT,
};

#define FIRST_RESERVED	257

enum RESERVED {
    /* terminal symbols denoted by reserved words */
    TK_NONE,
    TK_AND = FIRST_RESERVED,
    TK_BREAK,
    TK_DO,
    TK_ELSE,
    TK_ELSEIF,
    TK_END,
    TK_FALSE,
    TK_FOR,
    TK_FUNCTION,
    TK_IF,
    TK_IN,
    TK_LOCAL,
    TK_NIL,
    TK_NOT,
    TK_OR,
    TK_REPEAT,
    TK_RETURN,
    TK_THEN,
    TK_TRUE,
    TK_UNTIL,
    TK_WHILE,
    TK_RESERVED_COUNT,
    /* other terminal symbols */
    TK_CONCAT = TK_RESERVED_COUNT,
    TK_DOTS,
    TK_EQ,
    TK_GE,
    TK_LE,
    TK_NE,
    TK_NUMBER,
    TK_NAME,
    TK_STRING,
    TK_EOS
};

#define NUM_RESERVED	(TK_RESERVED_COUNT - FIRST_RESERVED)

typedef std::bitset<MARKED_COUNT> lua_marked;
typedef double lua_Number;

struct TValue;
union GCObject;


struct GCheader{
    // GCObject* next;
    TVALUE_TYPE tt = LUA_TNIL;  // GC的时候使用，来判断类型
    lua_marked marked;

    virtual ~GCheader() {}
};

union Value {
    GCheader* gc = nullptr;  // 非内置类型，只存指针，需要GC
    void* p;
    lua_Number n;
    int b;
};


#ifdef _DEBUG
#define SET_DEBUG_NAME(t, s) if (s) {(((t)->name) = (s));}
#define COPY_DEBUG_NAME(d, s) (((d)->name) = (s)->name)
#else
#define SET_DEBUG_NAME(t, s)
#define COPY_DEBUG_NAME(d, s)
#endif

// Value With Type
struct TValue {
#ifdef _DEBUG
    std::string name;
#endif
    Value value;
    TVALUE_TYPE tt = LUA_TNIL;

    void setvalue(const lua_Number n) {
        tt = LUA_TNUMBER;
        value.n = n;
    }
    void setvalue(void* p) {
        tt = LUA_TLIGHTUSERDATA;
        value.p = p;
    }
    void setvalue(const bool b) {
        tt = LUA_TBOOLEAN;
        value.b = b;
    }
    template<typename T>
    void setvalue(const T* x) {
        tt = TVALUE_TYPE(T::t);
        value.gc = const_cast<T*>(x);
    }

    TValue() {}
    TValue(lua_Number n) { setvalue(n); }
    TValue(void* p) { setvalue(p); }
    TValue(const bool b) { setvalue(b); }
    template<typename T>
    TValue(const T* x) { setvalue(x); }

    ~TValue() {}

    bool operator==(const TValue& k) const;
};

struct TString: GCheader {
    RESERVED     reserved = TK_NONE;      // 字符串为系统保留标识符时，这里不为0
    unsigned int hash     = 0;
    std::string  s;

    enum { t = LUA_TSTRING };

    TString() { tt = LUA_TSTRING; }
};

inline size_t keyhash(const TValue& k) {
    switch (k.tt) {
    case LUA_TNUMBER: {
        return (size_t)k.value.n;
    }
    case LUA_TSTRING: {
        TString* ts = (TString*)k.value.gc;
        return std::hash<std::string>{}(ts->s);
    }
    default: {
        return 0;
    }
    }
}

inline bool TValue::operator==(const TValue& k) const {
    return keyhash(*this) == keyhash(k);
}

struct KeyFunction {
    size_t operator()(const TValue& k) const {
        return keyhash(k);
    }
};

struct Table : GCheader {
    Table* metatable = nullptr;
    std::vector<TValue> array;
    std::unordered_map<const TValue, TValue, KeyFunction> node;

    enum { t = LUA_TTABLE };

    TValue* setstr(lua_State* L, const TString* key);
    TValue* set(lua_State* L, const TValue* key);

    const TValue* getnum(int key);
    const TValue* getstr(const TString* key);
    const TValue* get(const TValue* key);
};

struct Closure : GCheader {
    lu_byte   isC     = false;
    GCObject* gclist  = nullptr;
    Table*    env     = nullptr;

    enum { t = LUA_TFUNCTION };
};

struct UpVal : GCheader {
    TValue* v = nullptr;  /* points to stack or to its own value */
    union Practical {
        TValue value;  /* the value (when closed) */
        struct {  /* double linked list (when open) */
            struct UpVal* prev = nullptr;
            struct UpVal* next = nullptr;
        } l;

        ~Practical() {}
    } u;
};

// 包含操作和数据的完整可执行模块
struct Proto : GCheader {
    std::vector<TValue>      k;            /* 被该函数引用到的常量 */
    std::vector<Instruction> code;         /* 指令列表 */
    std::vector<int>         lineinfo;     /* 指令列表中每个指令所在的line */
    std::vector<Proto*>      p;            /* 函数内嵌套函数 */

    enum { t = LUA_TPROTO };
};

// C函数中的指令和数据都在代码段数据段中，只需要一个函数指针入口即可
struct CClosure : Closure {
    lua_CFunction       f = nullptr;    // 函数指针
    std::vector<TValue> upvalue;

    CClosure() { isC = true; }
};

// Lua函数的指令和数据需要自行管理，因此需要一个Proto来存储
struct LClosure : Closure {
    Proto*              p = nullptr;    // 函数原型
    std::vector<UpVal*> upvals;

    LClosure() { isC = false; }
};

inline bool ttisnumber(TValue* obj) {
    return obj->tt == LUA_TNUMBER;
}
inline bool ttisstring(TValue* obj) {
    return obj->tt == LUA_TSTRING;
}

void setnilvalue(TValue* obj _DECL);
void setnvalue(TValue* obj, const lua_Number n _DECL);
void setpvalue(TValue* obj, void* p _DECL);
void setbvalue(TValue* obj, const bool b _DECL);
void setobj(TValue* desc, const TValue* src);

const TValue luaO_nilobject_;
#define luaO_nilobject (&luaO_nilobject_)

// setsvalue
// sethvalue
// setclvalue
// setptvalue

template<typename T>
void setgcvalue(TValue* obj, const T* x _DECL) {
    obj->setvalue(x);
    SET_DEBUG_NAME(obj, debug);
}

int luaO_str2d(const char* s, lua_Number* result);