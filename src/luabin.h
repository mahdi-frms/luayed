#ifndef LUABIN_h
#define LUABIN_h

#include "luadef.h"

enum Instruction
{
    IAdd = 0x10,
    ISub = 0x11,
    IMult = 0x12,
    IFlrDiv = 0x13,
    IFltDiv = 0x14,
    IMod = 0x15,
    IPow = 0x16,
    IConcat = 0x17,
    IBOr = 0x18,
    IBAnd = 0x19,
    IBXor = 0x1a,
    ISHR = 0x1b,
    ISHL = 0x1c,

    ILength = 0x20,
    INegate = 0x21,
    INot = 0x22,
    IBNot = 0x23,

    IEq = 0x30,
    INe = 0x31,
    IGe = 0x32,
    IGt = 0x33,
    ILe = 0x34,
    ILt = 0x35,

    ITGet = 0x40,
    ITSet = 0x41,
    ITNew = 0x42,
    IGGet = 0x43,
    IGSet = 0x44,
    INil = 0x45,
    ITrue = 0x46,
    IFalse = 0x47,

    IUPush = 0x50,
    IUPop = 0x51,

    ITList = 0xc0,
    IRet = 0xc2,

    ICall = 0xd0,
    IVargs = 0xd4,
    IJmp = 0xd6,
    ICjmp = 0xd8,

    IConst = 0xe0,
    IFConst = 0xe2,

    ILocal = 0xf0,
    ILStore = 0xf2,
    IBLocal = 0xf4,
    IBLStore = 0xf6,
    IUpvalue = 0xf8,
    IUStore = 0xfa,

    IPop = 0xfe
};

struct Upvalue
{
    fidx_t fidx;
    size_t offset;
    size_t hidx;

    Upvalue(fidx_t fidx, size_t offset, size_t hidx);
    friend bool operator==(const Upvalue &l, const Upvalue &r);
};

struct Opcode
{
    lbyte count;
    lbyte bytes[5];

    Opcode(lbyte op, size_t idx);
    Opcode(lbyte op, size_t idx1, size_t idx2);
    Opcode(lbyte op);
};

string binary_stringify(lbyte *text, size_t codelen);

#define iadd IAdd
#define isub ISub
#define imult IMult
#define iflrdiv IFlrDiv
#define ifltdiv IFltDiv
#define imod IMod
#define ipow IPow
#define iconcat IConcat
#define ibor IBOr
#define iband IBAnd
#define ibxor IBXor
#define ishr ISHR
#define ishl ISHL
#define ilength ILength
#define inegate INegate
#define inot INot
#define ibnot IBNot
#define ieq IEq
#define ine INe
#define ige IGe
#define igt IGt
#define ile ILe
#define ilt ILt
#define itget ITGet
#define itset ITSet
#define itnew ITNew
#define igget IGGet
#define igset IGSet
#define inil INil
#define itrue ITrue
#define ifalse IFalse
#define iupush IUPush
#define iupop IUPop
#define itlist(A) Opcode(ITList, A)
#define iret(A) Opcode(IRet, A)
#define icall(A, B) Opcode(ICall, A, B)
#define ivargs(A) Opcode(IVargs, A)
#define ijmp(A) Opcode(IJmp, A)
#define icjmp(A) Opcode(ICjmp, A)
#define iconst(A) Opcode(IConst, A)
#define ifconst(A) Opcode(IFConst, A)
#define ilocal(A) Opcode(ILocal, A)
#define ilstore(A) Opcode(ILStore, A)
#define iblocal(A) Opcode(IBLocal, A)
#define iblstore(A) Opcode(IBLStore, A)
#define iupvalue(A) Opcode(IUpvalue, A)
#define iustore(A) Opcode(IUStore, A)
#define ipop(A) Opcode(IPop, A)

#endif