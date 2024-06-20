#pragma once

#include "llimits.h"

// 无效register的值，0x11111111
#define NO_REG		((1<<8)-1)

inline OpCode GET_OPCODE(const Instruction& i) {
    return i.op;
}

inline int GETARG_A(const Instruction& i) {
    return i.a;
}
inline void SETARG_A(Instruction& i, int u) {
    i.a = u;
}
inline int GETARG_B(const Instruction& i) {
    return i.b;
}
inline void SETARG_B(Instruction& i, int u) {
    i.b = u;
}
inline int GETARG_C(const Instruction& i) {
    return i.c;
}
inline void SETARG_C(Instruction& i, int u) {
    i.c = u;
}
inline int GETARG_Bx(const Instruction& i) {
    return i.bc;
}

inline Instruction CREATE_ABC(OpCode o, int a, int b, int c) {
    Instruction i{ o, a, b, c };
    return i;
}

inline Instruction CREATE_ABx(OpCode o, int a, int bc) {
    //Instruction i{ o, a, bc };
    Instruction i;
    i.op = o;
    i.a = a;
    i.bc = bc;
    return i;
}