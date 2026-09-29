// PlatformTypes.h
//
// Neutral entry point for the small set of MFC/Windows-SDK types the RMT
// engine and "GUI-shared" sources rely on (CString, BOOL/WORD/DWORD,
// HWND/HINSTANCE, COLORREF/RGB, CDC and friends).
//
//   - When building with real MFC (RMT_HAS_MFC defined, see src/CMakeLists.txt
//     -> MFC_AVAILABLE), this resolves to exactly <afxwin.h>, i.e. the
//     original, untouched Windows/MSVC behaviour.
//   - Otherwise (Linux/GCC, no MFC), it resolves to MfcTypes.h, a minimal
//     replacement good enough to compile (and link) the engine outside of
//     MFC.
//
// Core files that used to '#include "StdAfx.h"' directly now include this
// header instead; General.h (included by virtually every core header) also
// includes this header so that any file reaching StdAfx.h indirectly still
// sees the right types automatically.

#pragma once

// MSVC builds use MFC unless they build the Qt frontend: the Visual Studio
// project (Rmt.vcxproj) does not define RMT_HAS_MFC, CMake does (MFC_AVAILABLE)
#if !defined(RMT_HAS_MFC) && defined(_MSC_VER) && !defined(RMT_QT_GUI)
#define RMT_HAS_MFC
#endif

#ifdef RMT_HAS_MFC
#include "StdAfx.h"
#else
#include "MfcTypes.h"
#endif
