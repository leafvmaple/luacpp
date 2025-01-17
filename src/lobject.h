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
    FIXEDBIT, // ����GC���ؼ��֣�
    SFIXEDBIT, // Super fixed, ��Thread��ʹ�ã�ȷ��luaC_freeall��ʱ��Ҳ����GC

    MARKED_COUNT,
};

constexpr int FIRST_RESERVED = 257;

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

constexpr int NUM_RESERVED = TK_RESERVED_COUNT - FIRST_RESERVED;

struct global_State;
struct GCheader;

using lua_GCList = std::list<GCheader*>;
using lua_GCMap = std::unordered_map<size_t, lua_GCList>;

using lua_Number = double;

template<typename T>
struct lua_stack {
    void settop(const T* p) {
        s.resize(p - &s.front());
    }
    auto pop(int n = 1) {
        const T ret = *(top() - n);
        expand(-n);
        return ret;
    }
    void expand(int diff) {
        s.resize(s.size() + diff);
    }
    auto top() {
        return s._Unchecked_end();
    }
    void erase(size_t index) {
        s.erase(s.begin() + index);
    }
    void erase(const T* p) {
        erase(p - s.data());
    }
    template<typename _Ty>
    auto& emplace_back(_Ty v, const char* debug) {
        return s.emplace_back(T(v, debug));
    }
    // Bridge
    auto& back() {
        return s.back();
    }
    auto& operator[](int index) {
        if (index < 0)
            index = static_cast<int>(s.size() + index);
        return s[index];
    }
    auto size() {
        return s.size();
    }
    void resize(size_t size) {
        s.resize(size);
    }
    void push_back(const T& value) {
        s.push_back(value);
    }
    auto& emplace_back(T&& value) {
        return s.emplace_back(value);
    }
    auto& emplace_back() {
        return s.emplace_back();
    }

    std::vector<T> s;
};

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
    void tootherwhite();

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
    lua_State* L = nullptr;
    size_t _size = 0;

    virtual void markobject(global_State* g);
    virtual int traverse(global_State* g);
    virtual size_t hash() const { return 0; }

    void link(lua_State* L);
    void trymark(global_State* g);

    void setname(const char* debug) {
#ifdef _DEBUG
        name = debug;
#endif
    }

    virtual ~GCheader() {}
};

struct TString;
struct Table;
struct Closure;

// Value With Type
struct TValue {
    union {
        GCheader* gc = nullptr;  // ���������ͣ�ֻ��ָ�룬��ҪGC
        void* p;
        lua_Number n;
        int b;

        TString* ts;
        Table* h;
        Closure* cl;
    };
    TVALUE_TYPE tt = LUA_TNIL;

#ifdef _DEBUG
    std::string name;
#endif

    void setvalue(nullptr_t) {
        tt = LUA_TNIL;
        gc = nullptr;
    }
    void setvalue(const lua_Number _n) {
        tt = LUA_TNUMBER;
        n = _n;
    }
    void setvalue(void* _p) {
        tt = LUA_TLIGHTUSERDATA;
        p = _p;
    }
    void setvalue(const bool _b) {
        tt = LUA_TBOOLEAN;
        b = _b;
    }
    template<typename T>
    void setvalue(const T* x) {
        tt = TVALUE_TYPE(T::t);
        gc = const_cast<T*>(x);
#ifdef _DEBUG
        if (tt >= LUA_TSTRING)
            gc->name = name;
#endif
    }

    template<typename T>
    void setvalue(T x, const char* const debug) {
#ifdef _DEBUG
        name = debug;
#endif
        setvalue(x);
    }

    int tostring(lua_State* L);

    size_t hash() const {
        switch (tt) {
        case LUA_TNUMBER: return static_cast<size_t>(n);
        default: return gc->hash();
        }
    }

    inline bool isnil() const {
        return tt == LUA_TNIL;
    }
    inline bool isnumber() const {
        return tt == LUA_TNUMBER;
    }
    inline bool isstring() const {
        return tt == LUA_TSTRING;
    }

    template<typename T>
    TValue& operator=(const T x) {
        setvalue(x);
        return *this;
    }

    TValue() { setvalue(nullptr); }

    template<typename T, typename... Args>
    explicit TValue(const T x, Args... args) { setvalue(x, args...); }

    void markvalue(global_State* g);
};

struct TString : GCheader {
    RESERVED     reserved = TK_NONE;      // �ַ���Ϊϵͳ������ʶ��ʱ��Ϊ0
    std::string  s;

    enum { t = LUA_TSTRING };

    TString(lua_State* L, const char* str, size_t l);
    ~TString();

    size_t hash() const override {
        return std::hash<std::string>::_Do_hash(s);
    }

    // ���ò���GC
    void fix() {
        marked.set(FIXEDBIT);
    }
};

struct Table : GCheader {
    Table* metatable = nullptr;
    std::vector<TValue> array;
    std::unordered_map<size_t, std::pair<TValue, TValue>> node;

    GCheader* gclist = nullptr;

    enum { t = LUA_TTABLE };

    Table(lua_State* _L, int narray, int nhash);
    ~Table();

    virtual int traverse(global_State* g);

    TValue* setnum(lua_State* L, int key);
    TValue* setstr(lua_State* L, const TString* key);
    TValue* set(lua_State* L, const TValue* key);

    const TValue& operator[](const TValue* key) const {
        return *get(key);
    }
    const TValue& operator[](lua_Number key) const {
        return *getnum(static_cast<int>(key));
    }
    const TValue& operator[](const TString* key) const {
        return *getstr(key);
    }

    TValue& operator[](const TValue* key) {
        return *set(L, key);
    }
    TValue& operator[](lua_Number key) {
        return *setnum(L, static_cast<int>(key));
    }
    TValue& operator[](const TString* key) {
        return *setstr(L, key);
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

// �������������ݵ�������ִ��ģ��
struct Proto : GCheader {
    struct Code {
        Instruction code;
        int p;
    };
    
    std::vector<TValue>      k; /* ���ú������õ��ĳ��� */
    std::vector<Code> codeinfo; /* ָ���б� */
    std::vector<Proto*>      p; /* ������Ƕ�׺��� */

    enum { t = LUA_TPROTO };

    Proto(lua_State* L);
    ~Proto();
};

// C�����е�ָ������ݶ��ڴ�������ݶ��У�ֻ��Ҫһ������ָ����ڼ���
struct CClosure : Closure {
    lua_CFunction       f = nullptr;    // ����ָ��
    std::vector<TValue> upvalue;

    CClosure(lua_State* L, int nelems, Table* e);
    ~CClosure();

    virtual int precall(lua_State* L, TValue* func, int nresults);
};

// Lua������ָ���������Ҫ���й��������Ҫһ��Proto���洢
struct LClosure : Closure {
    Proto*              p = nullptr;    // ����ԭ��
    std::vector<UpVal*> upvals;

    LClosure(lua_State* L, int nelems, Table* e);
    ~LClosure();

    virtual int precall(lua_State* L, TValue* func, int nresults);
    void execute(lua_State* L, int nexeccalls);
};

extern const TValue luaO_nilobject_;
constexpr const TValue* luaO_nilobject = &luaO_nilobject_;

int luaO_str2d(const char* s, lua_Number* result);