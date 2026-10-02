/*
    The 6502 of the program: the built-in emulation of emu/Cpu6502.
*/

#pragma once

#include "PlatformTypes.h"

extern BOOL volatile g_is6502;
extern CString g_about6502;

class C6502 {
public:
    typedef unsigned short Address;
    typedef unsigned char Byte;
    typedef Byte Register;
    typedef int CycleCount;
    typedef int ClockFrequency;

    static int Init(byte* memory);
    static void DeInit();

    // The cycles parameter, is the maximum number of cycles to run. The method call reduces this value by the number of cycles actually run before RTS.
    static void JSR(Address& adr, Register& a, Register& x, Register& y, CycleCount& cycles);
};
