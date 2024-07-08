#pragma once

#include "llimits.h"

// 无效register的值，0x11111111
#define NO_REG		((1<<8)-1)

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