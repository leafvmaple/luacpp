#pragma once

#include <limits.h>
#include <stddef.h>

#include "lua.h"

using lu_byte = unsigned char;
using lu_int32 = unsigned int;
using l_mem = ptrdiff_t;
using lu_mem = size_t;

constexpr int MINSTRTABSIZE = 32;

enum OpCode {
    /*----------------------------------------------------------------------
    name		args	description
    ------------------------------------------------------------------------*/
    OP_MOVE,/*	A B	R(A) := R(B)					*/
    OP_LOADK,/*	A Bx	R(A) := Kst(Bx)					*/
    OP_LOADBOOL,/*	A B C	R(A) := (Bool)B; if (C) pc++			*/
    OP_LOADNIL,/*	A B	R(A) := ... := R(B) := nil			*/
    OP_GETUPVAL,/*	A B	R(A) := UpValue[B]				*/

    OP_GETGLOBAL,/*	A Bx	R(A) := Gbl[Kst(Bx)]				*/
    OP_GETTABLE,/*	A B C	R(A) := R(B)[RK(C)]				*/

    OP_SETGLOBAL,/*	A Bx	Gbl[Kst(Bx)] := R(A)				*/
    OP_SETUPVAL,/*	A B	UpValue[B] := R(A)				*/
    OP_SETTABLE,/*	A B C	R(A)[RK(B)] := RK(C)				*/

    OP_NEWTABLE,/*	A B C	R(A) := {} (size = B,C)				*/

    OP_SELF,/*	A B C	R(A+1) := R(B); R(A) := R(B)[RK(C)]		*/

    OP_ADD,/*	A B C	R(A) := RK(B) + RK(C)				*/
    OP_SUB,/*	A B C	R(A) := RK(B) - RK(C)				*/
    OP_MUL,/*	A B C	R(A) := RK(B) * RK(C)				*/
    OP_DIV,/*	A B C	R(A) := RK(B) / RK(C)				*/
    OP_MOD,/*	A B C	R(A) := RK(B) % RK(C)				*/
    OP_POW,/*	A B C	R(A) := RK(B) ^ RK(C)				*/
    OP_UNM,/*	A B	R(A) := -R(B)					*/
    OP_NOT,/*	A B	R(A) := not R(B)				*/
    OP_LEN,/*	A B	R(A) := length of R(B)				*/

    OP_CONCAT,/*	A B C	R(A) := R(B).. ... ..R(C)			*/

    OP_JMP,/*	sBx	pc+=sBx					*/

    OP_EQ,/*	A B C	if ((RK(B) == RK(C)) ~= A) then pc++		*/
    OP_LT,/*	A B C	if ((RK(B) <  RK(C)) ~= A) then pc++  		*/
    OP_LE,/*	A B C	if ((RK(B) <= RK(C)) ~= A) then pc++  		*/

    OP_TEST,/*	A C	if not (R(A) <=> C) then pc++			*/
    OP_TESTSET,/*	A B C	if (R(B) <=> C) then R(A) := R(B) else pc++	*/

    OP_CALL,/*	A B C	R(A), ... ,R(A+C-2) := R(A)(R(A+1), ... ,R(A+B-1)) */
    OP_TAILCALL,/*	A B C	return R(A)(R(A+1), ... ,R(A+B-1))		*/
    OP_RETURN,/*	A B	return R(A), ... ,R(A+B-2)	(see note)	*/

    OP_FORLOOP,/*	A sBx	R(A)+=R(A+2);
    if R(A) <?= R(A+1) then { pc+=sBx; R(A+3)=R(A) }*/
    OP_FORPREP,/*	A sBx	R(A)-=R(A+2); pc+=sBx				*/

    OP_TFORLOOP,/*	A C	R(A+3), ... ,R(A+2+C) := R(A)(R(A+1), R(A+2));
    if R(A+3) ~= nil then R(A+2)=R(A+3) else pc++	*/
    OP_SETLIST,/*	A B C	R(A)[(C-1)*FPF+i] := R(A+i), 1 <= i <= B	*/

    OP_CLOSE,/*	A 	close all variables in the stack up to (>=) R(A)*/
    OP_CLOSURE,/*	A Bx	R(A) := closure(KPROTO[Bx], R(A), ... ,R(A+n))	*/

    OP_VARARG/*	A B	R(A), R(A+1), ..., R(A+B-1) = vararg		*/
};

struct Instruction {
    OpCode op : 6;
    int a : 8;
    union
    {
        struct {
            int b : 9;
            int c : 9;
        };
        int bc : 18; // Bx
    };
};

// typedef lu_int32 Instruction;
