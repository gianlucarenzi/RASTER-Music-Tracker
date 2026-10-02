#include "C6502.h"

#include "PlatformTypes.h"
#include "emu/Cpu6502.h"

BOOL volatile g_is6502 = FALSE;
CString g_about6502;

int C6502::Init(byte* memory)
{
    // The built-in 6502 (emu/Cpu6502)
    {
        char *name, *author, *description;
        RmtBuiltin_C6502_About(&name, &author, &description);
        g_about6502.Format("%s\n%s\n%s", name, author, description);
    }

    RmtBuiltin_C6502_Initialise(memory);

    g_is6502 = 1;

    return 1;
}


void C6502::DeInit()
{
    g_is6502 = 0;
    g_about6502 = "No Atari 6502 CPU emulation.";
}


void C6502::JSR(C6502::Address& adr, C6502::Register& a, C6502::Register& x, C6502::Register& y, C6502::CycleCount& cycles)
{
    if (!g_is6502) return; // no 6502 emulation (see Init())
    RmtBuiltin_C6502_JSR(&adr, &a, &x, &y, &cycles);
}
