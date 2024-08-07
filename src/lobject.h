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
    FIXEDBIT, // 不会GC（关键字）
    SFIXEDBIT, // 主Thread，luaC_freeall的时候也不会GC

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

enum GC_STATE {
    GCSpause,
    GCSpropagate,
    GCSsweepstring,
    GCSsweep,
    GCSfinalize,
};

#define NUM_RESERVED	(TK_RESERVED_COUNT - FIRST_RESERVED)

struct global_State;
struct GCheader;
struct TValue;
union GCObject;

typedef std::list<GCheader*> lua_GCList;
typedef std::unordered_map<size_t, lua_GCList> lua_GCHash;

typedef double lua_Number;

template<typename T>
struct lua_stack : std::vector<T> {
    void settop(const T* top) {
        // 为什么要加this->？
        this->resize(top - &this->front());
    }
    // TODO
    const T pop(int n = 1) {
        const T ret = this->operator[](this->size() - n);
        while (n--)
            this->pop_back();
        return ret;
    }
    void expand(int diff) {
        this->resize(this->size() + diff);
    }
    auto top() {
        return this->_Unchecked_end();
    }
};

void* operator new(std::size_t size, lua_State* L);
void operator delete(void* p, lua_State* L) noexcept;

struct lua_Marked {
    std::bitset<MARKED_COUNT> bit;

    bool operator[](size_t index) const {
        return bit[index];
    }

    void set(size_t index, bool value = true) {
        bit.set(index, value);
    }

    void towhite(global_State* g, bool reset = true);

    void togray();
    void toblack();

    bool iswhite() const;
    bool isblack() const;
    bool isgray() const;

    int isdead(global_State* g) const;

    void maskmarks(global_State* g);
};

struct GCheader {
#ifdef _DEBUG
    std::string name;
#endif
    lua_Marked marked;

    virtual void markobject(global_State* g);
    virtual int traverse(global_State* g);

    void link(lua_State* L);
    void trymark(global_State* g);

    virtual ~GCheader() {}
};

#ifdef _DEBUG
#define _SET_DEBUG_NAME if (debug) { \
    ((this->name) = (debug));        \
    if (tt >= LUA_TSTRING) ((gc->name) = (debug));  \
}
#else
#define _SET_DEBUG_NAME
#endif

// Value With Type
struct TValue {
    union {
        GCheader* gc = nullptr;  // 非内置类型，只存指针，需要GC
        void* p;
        lua_Number n;
        int b;
    };
    TVALUE_TYPE tt = LUA_TNIL;

#ifdef _DEBUG
    std::string name;
#endif

    void setnil(_NAME) { 
        tt = LUA_TNIL;
#ifdef _DEBUG
        gc = nullptr; // Debug 模式下清理数据
#endif
    _SET_DEBUG_NAME
    }
    void setvalue(const lua_Number _n, _NAME) { 
        tt = LUA_TNUMBER;
        n = _n;
    _SET_DEBUG_NAME
    }
    void setvalue(void* _p, _NAME) { 
        tt = LUA_TLIGHTUSERDATA;
        p = _p;
    _SET_DEBUG_NAME
    }
    void setvalue(const bool _b, _NAME) {
        tt = LUA_TBOOLEAN;
        b = _b;
    _SET_DEBUG_NAME
    }
    template<typename T>
    void setvalue(const T* x, _NAME) {
        tt = TVALUE_TYPE(T::t);
        gc = const_cast<T*>(x);
    _SET_DEBUG_NAME
    }

    int tostring(lua_State* L);

    TValue(_NAME) { setnil(debug); }
    explicit TValue(lua_Number n, _NAME) { setvalue(n, debug); }
    explicit TValue(void* p, _NAME) { setvalue(p, debug); }
    explicit TValue(const bool b, _NAME) { setvalue(b, debug); }
    template<typename T>
    explicit TValue(const T* x, _NAME) { setvalue(x, debug); }

    ~TValue() {}

    void markvalue(global_State* g);
};

struct TString : GCheader {
    RESERVED     reserved = TK_NONE;      // 字符串为系统保留标识符时不为0
    std::string  s;

    enum { t = LUA_TSTRING };

    TString(lua_State* L, const char* str, size_t l);

    // 设置不会GC
    void fix() {
        marked.set(FIXEDBIT);
    }
};

inline size_t keyhash(const TValue& k) {
    switch (k.tt) {
    case LUA_TNUMBER: {
        return (size_t)k.n;
    }
    case LUA_TSTRING: {
        TString* ts = (TString*)k.gc;
        return std::hash<std::string>{}(ts->s);
    }
    default: {
        return 0;
    }
    }
}

struct Table : GCheader {
    Table* metatable = nullptr;
    std::vector<TValue> array;
    std::unordered_map<size_t, std::pair<TValue, TValue>> node;

    lua_State* L = nullptr;
    GCheader* gclist = nullptr;

    enum { t = LUA_TTABLE };

    Table(lua_State* _L, int narray, int nhash);

    virtual int traverse(global_State* g);

    TValue* setstr(lua_State* L, const TString* key);
    TValue* set(lua_State* L, const TValue* key);
    template<typename T>
    void set(lua_State* L, const TValue* key, T value) {
        TValue* idx = set(L, key);
        idx->setvalue(value);
    }

    const TValue& operator[](const TValue* key) const {
        return *get(key);
    }

    TValue& operator[](const TValue* key) {
        return *set(L, key);
    }

    const TValue* getnum(int key) const;
    const TValue* getstr(const TString* key) const;
    const TValue* get(const TValue* key) const;
};

struct Closure : GCheader {
    Table* env = nullptr;

    enum { t = LUA_TFUNCTION };

    virtual int precall(lua_State* L, TValue* func, int nresults) = 0;
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

    Proto(lua_State* L);
};

// C函数中的指令和数据都在代码段数据段中，只需要一个函数指针入口即可
struct CClosure : Closure {
    lua_CFunction       f = nullptr;    // 函数指针
    std::vector<TValue> upvalue;

    CClosure(lua_State* L, int nelems, Table* e);

    virtual int precall(lua_State* L, TValue* func, int nresults);
};

// Lua函数的指令和数据需要自行管理，因此需要一个Proto来存储
struct LClosure : Closure {
    Proto*              p = nullptr;    // 函数原型
    std::vector<UpVal*> upvals;

    LClosure(lua_State* L, int nelems, Table* e);

    virtual int precall(lua_State* L, TValue* func, int nresults);
    void execute(lua_State* L, int nexeccalls);
};

inline bool ttisnil(TValue* obj) {
    return obj->tt == LUA_TNIL;
}

inline bool ttisnumber(TValue* obj) {
    return obj->tt == LUA_TNUMBER;
}
inline bool ttisstring(TValue* obj) {
    return obj->tt == LUA_TSTRING;
}

const TValue luaO_nilobject_;
#define luaO_nilobject (&luaO_nilobject_)

int luaO_str2d(const char* s, lua_Number* result);