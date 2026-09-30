#include "C6502.h"

#include "PlatformTypes.h"
#include "emu/Cpu6502.h"

//#include "Global.h" // TODO Get rid of this

#include "C6502.h"

// DDL procedure pointers.
typedef void (*SA_C6502_Initialise_PROC)(BYTE*);
typedef int (*SA_C6502_JSR_PROC)(WORD*, BYTE*, BYTE*, BYTE*, int*);
typedef void (*SA_C6502_About_PROC)(char**, char**, char**);

SA_C6502_Initialise_PROC SA_C6502_Initialise;
SA_C6502_JSR_PROC SA_C6502_JSR;
SA_C6502_About_PROC SA_C6502_About;


HINSTANCE g_c6502_dll = NULL;
BOOL volatile g_is6502 = FALSE;
CString g_about6502;

extern HWND g_hwnd;

int C6502::Init(byte* memory)
{
    if (g_c6502_dll) { DeInit(); } //just in case

    // sa_c6502.dll when it is there (Windows), else the built-in 6502 (emu/Cpu6502)
    g_c6502_dll = LoadLibrary("sa_c6502.dll");
    if (g_c6502_dll) {
        CString wrn = "";

        SA_C6502_Initialise = (SA_C6502_Initialise_PROC)GetProcAddress(g_c6502_dll, "C6502_Initialise");
        if (!SA_C6502_Initialise) wrn += "C6502_Initialise\n";

        SA_C6502_JSR = (SA_C6502_JSR_PROC)GetProcAddress(g_c6502_dll, "C6502_JSR");
        if (!SA_C6502_JSR) wrn += "C6502_JSR\n";

        SA_C6502_About = (SA_C6502_About_PROC)GetProcAddress(g_c6502_dll, "C6502_About");
        if (!SA_C6502_About) wrn += "C6502_About\n";

        if (wrn != "") {
            MessageBox(g_hwnd, "Error:\n'sa_c6502.dll' is not compatible, the built-in 6502 is used.\nIncompatibility with:" + wrn, "C6502 library error", MB_ICONEXCLAMATION);
            FreeLibrary(g_c6502_dll);
            g_c6502_dll = NULL;
        }
    }
    if (!g_c6502_dll) {
        SA_C6502_Initialise = RmtBuiltin_C6502_Initialise;
        SA_C6502_JSR = RmtBuiltin_C6502_JSR;
        SA_C6502_About = RmtBuiltin_C6502_About;
    }

    //Text for About dialog
    {
        char *name, *author, *description;
        SA_C6502_About(&name, &author, &description);
        g_about6502.Format("%s\n%s\n%s", name, author, description);
    }

    SA_C6502_Initialise(memory);

    g_is6502 = 1;

    return 1;
}


void C6502::DeInit()
{
    g_is6502 = 0;
    SA_C6502_Initialise = NULL;
    SA_C6502_JSR = NULL;
    SA_C6502_About = NULL;

    if (g_c6502_dll) {
        FreeLibrary(g_c6502_dll);
        g_c6502_dll = NULL;
    }
    g_about6502 = "No Atari 6502 CPU emulation.";
}


void C6502::JSR(C6502::Address& adr, C6502::Register& a, C6502::Register& x, C6502::Register& y, C6502::CycleCount& cycles)
{
    if (!SA_C6502_JSR) return; // no 6502 emulation loaded (see Init())
    SA_C6502_JSR(&adr, &a, &x, &y, &cycles);
}
