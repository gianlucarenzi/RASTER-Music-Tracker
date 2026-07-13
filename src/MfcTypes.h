// MfcTypes.h
//
// Minimal, non-MFC replacement for the small subset of MFC/Windows-SDK
// types and free functions that the RMT engine (Song/Tracks/Instruments/
// C6502/Pokey/IO_*/GUI_Song/GUI_Instruments/GuiHelpers/...) happens to use.
//
// This header is only ever included when building WITHOUT real MFC
// (see PlatformTypes.h). It is never included by the genuine MFC files
// (Rmt.cpp, MainFrm.cpp, RmtDoc.cpp, RmtView.cpp, StdAfx.cpp, the *Dlg.cpp
// dialog implementations) which keep including <afxwin.h> exactly as
// before, so the Windows/MSVC build is completely unaffected by anything
// in this file.
//
// Everything here exists purely to make the engine + "GUI-shared" source
// files (GUI_Song.cpp, GUI_Instruments.cpp, GuiHelpers.cpp, TracksControl.cpp,
// ChannelControl.cpp, Global.cpp/h, ...) COMPILE AND LINK on a plain
// GCC/Linux toolchain. None of it attempts to reproduce real MFC/Win32
// behaviour or produce real rendering output - that is the job of a later
// migration phase (Qt backend). Where the original code takes a real
// branch (e.g. loading a DLL, creating a DirectSound buffer, opening a
// real dialog) the stub below simply reports "not available"/"cancelled"
// exactly like the real APIs already do on a machine without that
// resource - the existing error handling in the engine takes care of the
// rest.

#pragma once

#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <type_traits>
#include <ctime>
#include <system_error>
#include <strings.h>
#include <math.h>

// ---------------------------------------------------------------------------
// Basic Windows SDK typedefs
// ---------------------------------------------------------------------------

typedef int                BOOL;
typedef unsigned char       BYTE;
typedef unsigned char       byte;
typedef int                 boolean;
typedef unsigned short      WORD;
typedef unsigned long       DWORD;
typedef unsigned int        UINT;
typedef long                LONG;
typedef unsigned long       ULONG;
typedef uintptr_t           DWORD_PTR;
typedef uintptr_t           UINT_PTR;
typedef intptr_t            LONG_PTR;
typedef intptr_t            INT_PTR;
typedef void*               HANDLE;
typedef void*               HWND;
typedef void*               HINSTANCE;
typedef void*               HMODULE;
typedef void*               HCURSOR;
typedef void*               HICON;
typedef void*               HKEY;
typedef void*               HMENU;
typedef void*               HMIDIIN;
typedef void*               HMIDIOUT;
typedef void*               HGDIOBJ;
typedef void*               HRSRC;
typedef long                HRESULT;
typedef UINT_PTR            WPARAM;
typedef LONG_PTR            LPARAM;
typedef LONG_PTR            LRESULT;
typedef const char*         LPCTSTR;
typedef char*               LPTSTR;
typedef const char*         LPCSTR;
typedef char*               LPTSTR;
typedef char                TCHAR;
typedef void*               LPVOID;
typedef const void*         LPCVOID;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL nullptr
#endif

#define CALLBACK
#define WINAPI
#define AFX_MSG
#define afx_msg
#define DECLARE_MESSAGE_MAP()
#define DECLARE_DYNCREATE(x)
#define TEXT(x) x
#define _T(x) x

// Dialog / message-box results (subset actually used)
#define IDOK      1
#define IDCANCEL  2
#define IDYES     6
#define IDNO      7

// ---------------------------------------------------------------------------
// Virtual-key codes (subset actually referenced by GUI_Song.cpp/GUI_Instruments.cpp/
// GuiHelpers.cpp/TracksControl.cpp/ChannelControl.cpp), matching the real
// Win32 VK_* numeric values so the mapping stays meaningful once a real
// Qt/X11 keyboard backend is wired up in a later migration phase.
// (VK_BACKSPACE/VK_ENTER/VK_PAGE_UP/VK_PAGE_DOWN are already defined in
// Global.h with the same real values.)
// ---------------------------------------------------------------------------
#define VK_BACK      0x08
#define VK_TAB       0x09
#define VK_ESCAPE    0x1B
#define VK_SPACE     0x20
#define VK_PRIOR     0x21
#define VK_NEXT      0x22
#define VK_END       0x23
#define VK_HOME      0x24
#define VK_LEFT      0x25
#define VK_UP        0x26
#define VK_RIGHT     0x27
#define VK_DOWN      0x28
#define VK_INSERT    0x2D
#define VK_DELETE    0x2E
#define VK_0 0x30
#define VK_1 0x31
#define VK_2 0x32
#define VK_3 0x33
#define VK_4 0x34
#define VK_5 0x35
#define VK_6 0x36
#define VK_7 0x37
#define VK_8 0x38
#define VK_9 0x39
#define VK_A 0x41
#define VK_B 0x42
#define VK_C 0x43
#define VK_D 0x44
#define VK_E 0x45
#define VK_F 0x46
#define VK_G 0x47
#define VK_H 0x48
#define VK_I 0x49
#define VK_J 0x4A
#define VK_K 0x4B
#define VK_L 0x4C
#define VK_M 0x4D
#define VK_N 0x4E
#define VK_O 0x4F
#define VK_P 0x50
#define VK_Q 0x51
#define VK_R 0x52
#define VK_S 0x53
#define VK_T 0x54
#define VK_U 0x55
#define VK_V 0x56
#define VK_W 0x57
#define VK_X 0x58
#define VK_Y 0x59
#define VK_Z 0x5A
#define VK_MULTIPLY  0x6A
#define VK_ADD       0x6B
#define VK_SUBTRACT  0x6D
#define VK_DIVIDE    0x6F
#define VK_F1  0x70
#define VK_F2  0x71
#define VK_F3  0x72
#define VK_F4  0x73
#define VK_CAPITAL   0x14
#define VK_OEM_PLUS  0xBB
#define VK_OEM_MINUS 0xBD

#define MB_OK              0x00000000L
#define MB_OKCANCEL        0x00000001L
#define MB_YESNO           0x00000004L
#define MB_YESNOCANCEL     0x00000003L
#define MB_ICONERROR       0x00000010L
#define MB_ICONSTOP        0x00000010L
#define MB_ICONWARNING     0x00000030L
#define MB_ICONEXCLAMATION 0x00000030L
#define MB_ICONQUESTION    0x00000020L
#define MB_ICONINFORMATION 0x00000040L

// ---------------------------------------------------------------------------
// COLORREF / RGB
// ---------------------------------------------------------------------------

typedef uint32_t COLORREF;
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

// ---------------------------------------------------------------------------
// CString - minimal std::string-backed replacement.
//
// Implements exactly the ~20 methods/operators verified (by grep) to be in
// use across the engine + GUI-shared source files. Format() is implemented
// as a variadic *template* (not real C varargs) precisely so that passing
// a CString/std::string argument for a "%s" placeholder works correctly
// (it gets converted to a plain const char* first) - real MFC gets away
// with passing CString by value into "..." only because its internal
// representation is a single pointer; we don't rely on that trick.
// ---------------------------------------------------------------------------

class CString {
public:
    CString() = default;
    CString(const CString&) = default;
    CString(CString&&) = default;
    CString& operator=(const CString&) = default;
    CString& operator=(CString&&) = default;

    CString(const char* s) : m_data(s ? s : "") {}
    CString(char c) : m_data(1, c) {}
    CString(const std::string& s) : m_data(s) {}

    CString& operator=(const char* s) { m_data = (s ? s : ""); return *this; }

    // --- printf-style formatting -------------------------------------------------
    template <typename... Args>
    void Format(const char* fmt, Args... args) {
        m_data = FormatToString(fmt, ConvertArg(args)...);
    }
    template <typename... Args>
    void AppendFormat(const char* fmt, Args... args) {
        m_data += FormatToString(fmt, ConvertArg(args)...);
    }

    // --- queries -------------------------------------------------------------
    bool IsEmpty() const { return m_data.empty(); }
    int GetLength() const { return (int)m_data.length(); }
    const char* GetString() const { return m_data.c_str(); }
    char GetAt(int idx) const { return m_data.at((size_t)idx); }
    void SetAt(int idx, char ch) { m_data.at((size_t)idx) = ch; }
    void Empty() { m_data.clear(); }

    CString Left(int count) const {
        if (count < 0) count = 0;
        return CString(m_data.substr(0, (size_t)count));
    }
    CString Right(int count) const {
        if (count < 0) count = 0;
        size_t len = m_data.length();
        size_t n = (size_t)count > len ? len : (size_t)count;
        return CString(m_data.substr(len - n, n));
    }
    CString Mid(int first) const {
        if (first < 0) first = 0;
        if ((size_t)first >= m_data.length()) return CString();
        return CString(m_data.substr((size_t)first));
    }
    CString Mid(int first, int count) const {
        if (first < 0) first = 0;
        if (count < 0) count = 0;
        if ((size_t)first >= m_data.length()) return CString();
        return CString(m_data.substr((size_t)first, (size_t)count));
    }

    int Find(char ch, int start = 0) const {
        auto pos = m_data.find(ch, (size_t)start);
        return pos == std::string::npos ? -1 : (int)pos;
    }
    int Find(const char* sub, int start = 0) const {
        auto pos = m_data.find(sub, (size_t)start);
        return pos == std::string::npos ? -1 : (int)pos;
    }
    int ReverseFind(char ch) const {
        auto pos = m_data.rfind(ch);
        return pos == std::string::npos ? -1 : (int)pos;
    }

    int Replace(const char* oldStr, const char* newStr) {
        if (!oldStr || !*oldStr) return 0;
        int count = 0;
        size_t pos = 0;
        std::string oldS(oldStr), newS(newStr ? newStr : "");
        while ((pos = m_data.find(oldS, pos)) != std::string::npos) {
            m_data.replace(pos, oldS.length(), newS);
            pos += newS.length();
            count++;
        }
        return count;
    }
    int Replace(char oldCh, char newCh) {
        int count = 0;
        for (auto& c : m_data) { if (c == oldCh) { c = newCh; count++; } }
        return count;
    }

    CString& MakeUpper() {
        for (auto& c : m_data) c = (char)toupper((unsigned char)c);
        return *this;
    }
    CString& MakeLower() {
        for (auto& c : m_data) c = (char)tolower((unsigned char)c);
        return *this;
    }
    CString& Trim() {
        TrimLeftInPlace();
        TrimRightInPlace();
        return *this;
    }
    CString& TrimRight() { TrimRightInPlace(); return *this; }

    char* GetBuffer(int minLength = 0) {
        if (minLength > 0 && (size_t)minLength > m_data.size()) {
            m_data.resize((size_t)minLength);
        }
        return m_data.data();
    }
    void ReleaseBuffer(int newLength = -1) {
        if (newLength < 0) {
            m_data.resize(std::strlen(m_data.c_str()));
        } else {
            m_data.resize((size_t)newLength);
        }
    }

    int Compare(const char* other) const { return m_data.compare(other ? other : ""); }
    int CompareNoCase(const char* other) const {
        std::string a = m_data, b(other ? other : "");
        for (auto& c : a) c = (char)tolower((unsigned char)c);
        for (auto& c : b) c = (char)tolower((unsigned char)c);
        return a.compare(b);
    }

    // Loads a fixed, small resource-string table (see MfcTypes.cpp) instead
    // of a real Windows .rc string table.
    BOOL LoadString(UINT id);

    // --- conversions / operators ----------------------------------------
    operator const char*() const { return m_data.c_str(); }

    CString& operator+=(const CString& other) { m_data += other.m_data; return *this; }
    CString& operator+=(const char* s) { if (s) m_data += s; return *this; }
    CString& operator+=(char c) { m_data += c; return *this; }

    const std::string& str() const { return m_data; }

private:
    void TrimLeftInPlace() {
        size_t i = 0;
        while (i < m_data.size() && std::isspace((unsigned char)m_data[i])) i++;
        m_data.erase(0, i);
    }
    void TrimRightInPlace() {
        size_t n = m_data.size();
        while (n > 0 && std::isspace((unsigned char)m_data[n - 1])) n--;
        m_data.erase(n);
    }

    // Helpers for the Format() variadic template: convert CString/std::string
    // arguments to a plain const char* before handing them to vsnprintf-style
    // formatting; every other argument type is passed through unchanged.
    static const char* ConvertArg(const CString& s) { return s.GetString(); }
    static const char* ConvertArg(const std::string& s) { return s.c_str(); }
    template <typename T>
    static T ConvertArg(T v) { return v; }

    template <typename... Args>
    static std::string FormatToString(const char* fmt, Args... args) {
        int size = std::snprintf(nullptr, 0, fmt, args...);
        if (size < 0) return std::string();
        std::vector<char> buf((size_t)size + 1);
        std::snprintf(buf.data(), buf.size(), fmt, args...);
        return std::string(buf.data(), (size_t)size);
    }

    std::string m_data;
};

inline CString operator+(const CString& a, const CString& b) { return CString(a.str() + b.str()); }
inline CString operator+(const CString& a, const char* b) { return CString(a.str() + (b ? b : "")); }
inline CString operator+(const char* a, const CString& b) { return CString((a ? a : "") + b.str()); }
inline CString operator+(const CString& a, char b) { return CString(a.str() + b); }

inline bool operator==(const CString& a, const CString& b) { return a.str() == b.str(); }
inline bool operator==(const CString& a, const char* b) { return a.str() == (b ? b : ""); }
inline bool operator==(const char* a, const CString& b) { return b.str() == (a ? a : ""); }
inline bool operator!=(const CString& a, const CString& b) { return !(a == b); }
inline bool operator!=(const CString& a, const char* b) { return !(a == b); }
inline bool operator!=(const char* a, const CString& b) { return !(a == b); }
inline bool operator<(const CString& a, const CString& b) { return a.str() < b.str(); }

// ---------------------------------------------------------------------------
// GDI-ish minimal shims: CPoint / CRect / CPen / CBitmap / CBrush
// ---------------------------------------------------------------------------

class CPoint {
public:
    int x = 0, y = 0;
    CPoint() = default;
    CPoint(int x_, int y_) : x(x_), y(y_) {}
    CPoint operator+(const CPoint& o) const { return CPoint(x + o.x, y + o.y); }
    CPoint operator-(const CPoint& o) const { return CPoint(x - o.x, y - o.y); }
};

class CRect {
public:
    int left = 0, top = 0, right = 0, bottom = 0;
    CRect() = default;
    CRect(int l, int t, int r, int b) : left(l), top(t), right(r), bottom(b) {}
    void SetRect(int l, int t, int r, int b) { left = l; top = t; right = r; bottom = b; }
    int Width() const { return right - left; }
    int Height() const { return bottom - top; }
    CPoint TopLeft() const { return CPoint(left, top); }
    CPoint BottomRight() const { return CPoint(right, bottom); }
};

#define PS_SOLID 0
#define SRCCOPY  0x00CC0020L

class CPen {
public:
    CPen() = default;
    CPen(int style, int width, COLORREF color) : m_style(style), m_width(width), m_color(color) {}
private:
    int m_style = PS_SOLID;
    int m_width = 1;
    COLORREF m_color = 0;
};

class CBitmap {
public:
    CBitmap() = default;
};

class CBrush {
public:
    CBrush() = default;
    explicit CBrush(COLORREF) {}
};

// ---------------------------------------------------------------------------
// CDC - "null" device context. Only the handful of drawing primitives
// actually called from GUI_Song.cpp / GUI_Instruments.cpp / GuiHelpers.cpp /
// TracksControl.cpp / ChannelControl.cpp are implemented, all as no-ops.
// A real backend (QImage/QPainter) can implement this exact same interface
// later (Fase 2 of the migration plan) without touching any caller.
// ---------------------------------------------------------------------------

class CDC {
public:
    virtual ~CDC() = default;

    void FillSolidRect(int, int, int, int, COLORREF) {}
    void FillSolidRect(const CRect&, COLORREF) {}

    void MoveTo(int x, int y) { m_curX = x; m_curY = y; }
    void LineTo(int x, int y) { m_curX = x; m_curY = y; }

    void FrameRect(const CRect&, CBrush*) {}

    CPen* SelectObject(CPen* pen) {
        CPen* prev = m_currentPen;
        m_currentPen = pen;
        return prev;
    }

    void BitBlt(int, int, int, int, CDC*, int, int, DWORD) {}

private:
    int m_curX = 0, m_curY = 0;
    CPen* m_currentPen = nullptr;
};

// ---------------------------------------------------------------------------
// CWnd / CFrameWnd / simple control shims - just enough for MainFrm.h /
// EffectsDlg.h / importdlgs.h / exportdlgs.h / filenewdlg.h to declare
// their member variables, and for the (dead, headless) code paths that
// cast AfxGetMainWnd()/AfxGetApp()->GetMainWnd() to keep compiling.
// ---------------------------------------------------------------------------

struct CDataExchange;
struct MSG;
struct CREATESTRUCT;
typedef CREATESTRUCT* LPCREATESTRUCT;
struct MINMAXINFO;

class CWnd {
public:
    HWND m_hWnd = nullptr;
    virtual ~CWnd() = default;
    void SetWindowText(const char*) {}
    void Invalidate(BOOL = TRUE) {}
    BOOL GetSafeHwnd() const { return m_hWnd != nullptr; }
};

class CFrameWnd : public CWnd {
public:
    virtual ~CFrameWnd() = default;
};

class CStatic : public CWnd {};
class CEdit : public CWnd {};
class CButton : public CWnd {};
class CListBox : public CWnd {
public:
    void ResetContent() {}
    int AddString(const char*) { return 0; }
    void SetCurSel(int) {}
    int GetCurSel() const { return -1; }
};
class CComboBox : public CWnd {
public:
    void ResetContent() {}
    int AddString(const char*) { return 0; }
    void SetCurSel(int) {}
    int GetCurSel() const { return -1; }
};
class CToolBar : public CWnd {};
class CReBar : public CWnd {};
class CStatusBar : public CWnd {};
class CScrollBar : public CWnd {};
class CFont : public CWnd {};

// CDialog - never rendered in this build (headless engine target); DoModal()
// always reports "cancelled" which is exactly the code path every call site
// already has to handle for the "user dismissed the dialog" case.
class CDialog : public CWnd {
public:
    CDialog() = default;
    explicit CDialog(UINT /*nIDTemplate*/, CWnd* /*pParentWnd*/ = nullptr) {}
    virtual ~CDialog() = default;

    virtual INT_PTR DoModal() { return IDCANCEL; }
    virtual void DoDataExchange(CDataExchange*) {}
    virtual BOOL OnInitDialog() { return TRUE; }
    virtual void OnOK() {}
    virtual void OnCancel() {}
    virtual BOOL PreTranslateMessage(MSG*) { return FALSE; }
};

// ---------------------------------------------------------------------------
// CFileDialog - very small stand-in for the common Open/Save dialog used
// directly (not through a project header) in IO_Song.cpp. DoModal() always
// "cancels"; every call site already checks the return value.
// ---------------------------------------------------------------------------

struct OPENFILENAME_STUB {
    const char* lpstrTitle = nullptr;
    const char* lpstrInitialDir = nullptr;
    char* lpstrFile = nullptr;
    UINT nMaxFile = 0;
    int nFilterIndex = 0;
};

#define OFN_HIDEREADONLY      0x00000004L
#define OFN_OVERWRITEPROMPT   0x00000002L

class CFileDialog : public CDialog {
public:
    CFileDialog(BOOL bOpenFileDialog,
                LPCTSTR lpszDefExt = nullptr,
                LPCTSTR lpszFileName = nullptr,
                DWORD dwFlags = 0,
                LPCTSTR lpszFilter = nullptr,
                CWnd* pParentWnd = nullptr)
        : m_bOpen(bOpenFileDialog) {
        (void)lpszDefExt; (void)lpszFileName; (void)dwFlags; (void)lpszFilter; (void)pParentWnd;
    }

    INT_PTR DoModal() override { return IDCANCEL; }
    CString GetPathName() const { return m_path; }
    CString GetFileName() const { return m_path; }

    OPENFILENAME_STUB m_ofn;

private:
    BOOL m_bOpen = TRUE;
    CString m_path;
};

// ---------------------------------------------------------------------------
// CFile / CFileStatus / CByteArray - small but genuinely functional helpers
// (backed by std::filesystem / std::fstream), used by AtariBinaries.cpp and
// SongExporterTest.cpp.
// ---------------------------------------------------------------------------

struct CFileStatus {
    long m_size = 0;
};

class CFile {
public:
    enum OpenFlags { modeRead = 0, modeWrite = 1, modeReadWrite = 2, modeCreate = 0x1000 };

    CFile() = default;
    CFile(const char* path, UINT mode) : m_path(path ? path : "") {
        auto flags = (mode & modeWrite) ? (std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc)
                                         : (std::ios::in | std::ios::binary);
        m_stream.open(m_path, flags);
    }
    ~CFile() { Close(); }

    static BOOL GetStatus(const char* path, CFileStatus& status) {
        std::error_code ec;
        auto sz = std::filesystem::file_size(path ? path : "", ec);
        if (ec) return FALSE;
        status.m_size = (long)sz;
        return TRUE;
    }
    static BOOL Remove(const char* path) {
        std::error_code ec;
        return std::filesystem::remove(path ? path : "", ec) ? TRUE : FALSE;
    }

    UINT Read(void* buffer, UINT count) {
        if (!m_stream.is_open()) return 0;
        m_stream.read((char*)buffer, count);
        return (UINT)m_stream.gcount();
    }

    long GetLength() const {
        std::error_code ec;
        auto sz = std::filesystem::file_size(m_path, ec);
        return ec ? 0 : (long)sz;
    }

    CString GetFileName() const {
        std::filesystem::path p(m_path);
        return CString(p.filename().string());
    }

    void Close() { if (m_stream.is_open()) m_stream.close(); }

private:
    std::string m_path;
    std::fstream m_stream;
};

class CByteArray {
public:
    void SetSize(long size) { m_data.resize(size > 0 ? (size_t)size : 0); }
    byte* GetData() { return m_data.empty() ? nullptr : m_data.data(); }
    long GetSize() const { return (long)m_data.size(); }
private:
    std::vector<byte> m_data;
};

// ---------------------------------------------------------------------------
// CWinApp / AfxGetApp / AfxGetMainWnd / AfxMessageBox
//
// GetMainWnd() intentionally returns nullptr: several call sites
// (GUI_Song.cpp CSong::DrawInfo, GuiHelpers.cpp) were already written with
// a "headless mode" null-check for exactly this situation.
// ---------------------------------------------------------------------------

class CWinApp {
public:
    HINSTANCE m_hInstance = nullptr;
    CWnd* GetMainWnd() { return nullptr; }
};

inline CWinApp* AfxGetApp() {
    static CWinApp s_app;
    return &s_app;
}
inline CWnd* AfxGetMainWnd() { return nullptr; }
inline int AfxMessageBox(const char* text, UINT type = MB_OK, UINT = 0) {
    std::fprintf(stderr, "[AfxMessageBox] %s\n", text ? text : "");
    (void)type;
    return IDOK;
}

// A tiny non-MFC stand-in for CRmtApp (declared for real in Rmt.h, which is
// real-MFC-only and therefore not included in this build - see the
// RMT_HAS_MFC guard around '#include "Rmt.h"' in GUI_Song.cpp).
class CRmtApp {
public:
    CString GetVersionAndBuild() const;
};
extern CRmtApp g_app;

// ---------------------------------------------------------------------------
// MessageBox / Shell / misc Win32 free functions used directly by the
// engine. All are safe no-ops or best-effort equivalents; none of them is
// reachable from the RmtCoreTest smoke-test main(), so their exact
// behaviour is not runtime-critical for this build.
// ---------------------------------------------------------------------------

inline int MessageBox(HWND, const char* text, const char* caption, UINT type) {
    std::fprintf(stderr, "[MessageBox] %s: %s\n", caption ? caption : "", text ? text : "");
    (void)type;
    return IDOK;
}

inline void GetWindowRect(HWND, CRect* rect) { if (rect) *rect = CRect(0, 0, 0, 0); }

inline HMODULE LoadLibrary(const char*) { return nullptr; }
inline void* GetProcAddress(HMODULE, const char*) { return nullptr; }
inline BOOL FreeLibrary(HMODULE) { return TRUE; }

inline DWORD GetLastError() { return 0; }
inline DWORD FormatMessage(DWORD, LPCVOID, DWORD, DWORD, LPTSTR, DWORD, void*) { return 0; }
inline void LocalFree(void*) {}
inline HANDLE ShellExecute(HWND, const char*, const char*, const char*, const char*, int) {
    // Always report "success" (handle value > 32) so callers never take the
    // Win32-error-reporting branch (FormatMessage/GetLastError) which is not
    // meaningfully implementable here.
    return (HANDLE)(intptr_t)33;
}
#define SW_SHOWNORMAL 1
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100
#define FORMAT_MESSAGE_FROM_SYSTEM     0x00001000
#define FORMAT_MESSAGE_IGNORE_INSERTS  0x00000200
#define MAKELANGID(p, s) 0
#define LANG_NEUTRAL 0
#define SUBLANG_DEFAULT 0

inline void ZeroMemory(void* dst, size_t size) { std::memset(dst, 0, size); }

#define LOWORD(l) ((WORD)((DWORD_PTR)(l) & 0xffff))
#define HIWORD(l) ((WORD)(((DWORD_PTR)(l) >> 16) & 0xffff))

#define IDC_WAIT ((LPCTSTR)(intptr_t)32514)
inline HCURSOR LoadCursor(HINSTANCE, LPCTSTR) { return nullptr; }
inline HCURSOR SetCursor(HCURSOR) { return nullptr; }
inline BOOL EnableWindow(HWND, BOOL) { return TRUE; }
inline BOOL UpdateWindow(HWND) { return TRUE; }
inline int GetKeyState(int) { return 0; }

#define MAPVK_VK_TO_CHAR 2
inline UINT MapVirtualKeyEx(UINT, UINT, HANDLE) { return 0; }

inline void OutputDebugString(const char* s) { std::fprintf(stderr, "%s", s ? s : ""); }

inline int _strcmpi(const char* a, const char* b) { return strcasecmp(a, b); }

inline BOOL DeleteFile(const char* path) {
    std::error_code ec;
    return std::filesystem::remove(path ? path : "", ec) ? TRUE : FALSE;
}

// ---------------------------------------------------------------------------
// CTime - tiny wrapper around time_t/strftime, used by SongExporter
// (records the export timestamp and formats it for display).
// ---------------------------------------------------------------------------

class CTime {
public:
    CTime() = default;
    explicit CTime(std::time_t t) : m_time(t) {}

    static CTime GetCurrentTime() { return CTime(std::time(nullptr)); }

    CString Format(const char* fmt) const {
        char buf[128];
        std::tm tmVal{};
#if defined(_WIN32)
        localtime_s(&tmVal, &m_time);
#else
        localtime_r(&m_time, &tmVal);
#endif
        std::strftime(buf, sizeof(buf), fmt, &tmVal);
        return CString(buf);
    }

private:
    std::time_t m_time = 0;
};

// ---------------------------------------------------------------------------
// Legacy multimedia timer (winmm) - used only by SongTimer.cpp. Never
// actually starts a background callback here (headless build); the engine
// already guards all playback state via g_closeApplication/g_rmtroutine so
// simply never firing the callback is safe for a non-interactive build.
// ---------------------------------------------------------------------------

typedef void (CALLBACK* LPTIMECALLBACK)(UINT, UINT, DWORD_PTR, DWORD_PTR, DWORD_PTR);
#define TIME_PERIODIC 1
#define TIME_ONESHOT  0
inline UINT timeSetEvent(UINT, UINT, LPTIMECALLBACK, DWORD_PTR, UINT) { return 0; }
inline UINT timeKillEvent(UINT) { return 0; }

// ---------------------------------------------------------------------------
// Minimal DirectSound shim (legacy CXPokey/PokeyRederer.cpp renderer).
// DirectSoundCreate() always reports failure, exactly like it would on a
// real machine that has no DirectSound driver - InitSoundInternal() already
// handles that case by returning FALSE with a warning message, so none of
// the interface methods below are ever actually invoked at runtime, they
// only need to exist so the (dead) call sites link.
// ---------------------------------------------------------------------------

struct WAVEFORMATEX {
    WORD  wFormatTag = 0;
    WORD  nChannels = 0;
    DWORD nSamplesPerSec = 0;
    DWORD nAvgBytesPerSec = 0;
    WORD  nBlockAlign = 0;
    WORD  wBitsPerSample = 0;
    WORD  cbSize = 0;
};
#define WAVE_FORMAT_PCM 1

struct DSBUFFERDESC {
    DWORD dwSize = 0;
    DWORD dwFlags = 0;
    DWORD dwBufferBytes = 0;
    DWORD dwReserved = 0;
    WAVEFORMATEX* lpwfxFormat = nullptr;
};
struct DSBCAPS {
    DWORD dwSize = 0;
    DWORD dwFlags = 0;
    DWORD dwBufferBytes = 0;
};

#define DS_OK 0L
#define DS_ERR_GENERIC (-1L)
#define DSSCL_PRIORITY 2
#define DSBCAPS_PRIMARYBUFFER 1
#define DSBCAPS_GETCURRENTPOSITION2 0x00010000
#define DSBCAPS_LOCHARDWARE 0x00000004
#define DSBCAPS_LOCSOFTWARE 0x00000008
#define DSBCAPS_GLOBALFOCUS 0x00008000
#define DSBCAPS_STICKYFOCUS 0x00004000
#define DSBLOCK_FROMWRITECURSOR 0x00000001
#define DSBPLAY_LOOPING 0x00000001

class IDirectSoundBuffer {
public:
    HRESULT SetFormat(const WAVEFORMATEX*) { return DS_OK; }
    HRESULT GetCaps(DSBCAPS*) { return DS_OK; }
    HRESULT Lock(DWORD, DWORD, void** ppData1, DWORD* pSize1, void** ppData2, DWORD* pSize2, DWORD) {
        if (ppData1) *ppData1 = nullptr;
        if (pSize1) *pSize1 = 0;
        if (ppData2) *ppData2 = nullptr;
        if (pSize2) *pSize2 = 0;
        return DS_ERR_GENERIC;
    }
    HRESULT Unlock(void*, DWORD, void*, DWORD) { return DS_OK; }
    HRESULT Play(DWORD, DWORD, DWORD) { return DS_OK; }
    HRESULT Stop() { return DS_OK; }
    HRESULT GetCurrentPosition(DWORD* playCursor, DWORD* writeCursor) {
        if (playCursor) *playCursor = 0;
        if (writeCursor) *writeCursor = 0;
        return DS_OK;
    }
    HRESULT Release() { return DS_OK; }
};
typedef IDirectSoundBuffer* LPDIRECTSOUNDBUFFER;

class IDirectSound {
public:
    HRESULT SetCooperativeLevel(HWND, DWORD) { return DS_OK; }
    HRESULT CreateSoundBuffer(const DSBUFFERDESC*, LPDIRECTSOUNDBUFFER*, void*) { return DS_ERR_GENERIC; }
    HRESULT Release() { return DS_OK; }
};
typedef IDirectSound* LPDIRECTSOUND;

inline HRESULT DirectSoundCreate(void*, LPDIRECTSOUND*, void*) { return DS_ERR_GENERIC; }

// ---------------------------------------------------------------------------
// Minimal legacy MMIO/WAV shim (WaveFile.h/.cpp). mmioOpen() always
// "fails" (returns nullptr) and mmioCreateChunk() always reports an error,
// exactly like the real Win32 API would on a machine that couldn't create
// the output file - CWaveFile::OpenFile() already checks that and returns
// false, so WriteWave()/CloseFile() (and every other mmio* call below) are
// never actually reached; they only need to exist for the link to succeed.
// ---------------------------------------------------------------------------

typedef void* HMMIO;
typedef char* HPSTR;
typedef uint32_t FOURCC;

struct PCMWAVEFORMAT {
    WAVEFORMATEX wf;
    WORD wBitsPerSample = 0;
};

struct MMCKINFO {
    FOURCC ckid = 0;
    DWORD  cksize = 0;
    FOURCC fccType = 0;
    long   dwDataOffset = 0;
    DWORD  dwFlags = 0;
};

struct MMIOINFO {
    DWORD dwFlags = 0;
    char* pchNext = nullptr;
    char* pchEndWrite = nullptr;
};

#define MMIO_ALLOCBUF   0x00010000
#define MMIO_READWRITE  0x00000002
#define MMIO_CREATE     0x00001000
#define MMIO_CREATERIFF 0x00000020
#define MMIO_DIRTY      0x10000000
#define MMIO_WRITE      0x00000001
#define MMSYSERR_NOERROR 0
#define MMSYSERR_ERROR   1

inline FOURCC mmioFOURCC(char a, char b, char c, char d) {
    return (FOURCC)(BYTE)a | ((FOURCC)(BYTE)b << 8) | ((FOURCC)(BYTE)c << 16) | ((FOURCC)(BYTE)d << 24);
}
inline HMMIO mmioOpen(char*, void*, DWORD) { return nullptr; }
inline int mmioCreateChunk(HMMIO, MMCKINFO*, int) { return MMSYSERR_ERROR; }
inline int mmioWrite(HMMIO, const char*, int) { return 0; }
inline int mmioAscend(HMMIO, MMCKINFO*, int) { return MMSYSERR_ERROR; }
inline int mmioDescend(HMMIO, MMCKINFO*, const MMCKINFO*, int) { return MMSYSERR_ERROR; }
inline int mmioGetInfo(HMMIO, MMIOINFO*, int) { return MMSYSERR_ERROR; }
inline int mmioSetInfo(HMMIO, const MMIOINFO*, int) { return MMSYSERR_ERROR; }
inline long mmioSeek(HMMIO, long, int) { return -1; }
inline int mmioAdvance(HMMIO, MMIOINFO*, DWORD) { return MMSYSERR_ERROR; }
inline int mmioClose(HMMIO, int) { return MMSYSERR_NOERROR; }
