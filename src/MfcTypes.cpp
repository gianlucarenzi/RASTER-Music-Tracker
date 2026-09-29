// MfcTypes.cpp
//
// Out-of-line pieces of the MfcTypes.h shim: only compiled when building
// without real MFC (see PlatformTypes.h / RmtCoreTest CMake target).

#include "PlatformTypes.h"
#include "resource.h"
#include "RmtVersion.h"

#include <algorithm>
#include <cstring>
#include <vector>

// ---------------------------------------------------------------------------
// CString::LoadString() - stand-in for the Windows .rc string table.
// Only the resource IDs actually reached via LoadString() calls in the
// engine + GUI-shared source files are listed here (verified by grep):
//   IDS_RMTVERSION / IDS_RMT_VERSION (IO_Song.cpp)
// The other two (IDS_RMT_AUTHOR, IDS_RMT_REPOSITORY) are only used from
// AboutDialog.cpp, a real MFC dialog not part of this build, but are kept
// here too since they cost nothing and keep the table self-documenting.
// Text taken verbatim from Rmt.rc.
// ---------------------------------------------------------------------------

BOOL CString::LoadString(UINT id) {
    switch (id) {
    case IDS_RMT_VERSION: // == IDS_RMTVERSION
        m_data = RMT_VERSION_STRING;
        return TRUE;
    case IDS_RMT_AUTHOR:
        m_data = "by Radek Sterba, (c) Raster/C.P.U. (2002-2009), VinsCool (2021-2024), JAC! (2024-2026)";
        return TRUE;
    case IDS_RMT_REPOSITORY:
        m_data = "https://github.com/raster-atari-org/RASTER-Music-Tracker";
        return TRUE;
    default:
        m_data = "";
        return FALSE;
    }
}

// ---------------------------------------------------------------------------
// CRmtApp - tiny non-MFC stand-in (real CRmtApp lives in Rmt.h/Rmt.cpp,
// which are real-MFC-only). Mirrors CRmtApp::GetVersionAndBuild() from
// Rmt.cpp so the version string displayed by GUI_Song.cpp is identical.
// ---------------------------------------------------------------------------

CRmtApp g_app;

IRmtHost* g_rmtHost = nullptr;

CString CRmtApp::GetVersionAndBuild() const {
    CString version;
    version.LoadString(IDS_RMTVERSION);
    CString result;
    result.Format("%s (%s %s)", version, __DATE__, __TIME__);
    return result;
}

// ---------------------------------------------------------------------------
// CBitmap / CDC - software drawing (see MfcTypes.h)
// ---------------------------------------------------------------------------

static uint32_t rd32(const unsigned char* p) { return p[0] | (p[1] << 8) | (p[2] << 16) | ((uint32_t)p[3] << 24); }
static uint16_t rd16(const unsigned char* p) { return (uint16_t)(p[0] | (p[1] << 8)); }

// Windows .bmp (BITMAPFILEHEADER + BITMAPINFOHEADER), uncompressed,
// 1/4/8 bit with palette, 24/32 bit; bottom-up or top-down
BOOL CBitmap::LoadBMP(const unsigned char* d, size_t size) {
    if (!d || size < 54 || d[0] != 'B' || d[1] != 'M') return FALSE;
    uint32_t bits = rd32(d + 10), hdr = rd32(d + 14);
    int w = (int)rd32(d + 18), h = (int)rd32(d + 22);
    int bpp = rd16(d + 28);
    uint32_t comp = rd32(d + 30), used = rd32(d + 46);
    if (comp != 0 || w <= 0 || h == 0 || hdr < 40) return FALSE;
    bool topdown = h < 0;
    if (topdown) h = -h;
    if (bpp != 1 && bpp != 4 && bpp != 8 && bpp != 24 && bpp != 32) return FALSE;
    uint32_t ncol = bpp <= 8 ? (used ? used : 1u << bpp) : 0;
    const unsigned char* pal = d + 14 + hdr;
    size_t stride = (((size_t)w * bpp + 31) / 32) * 4;
    if (bits + stride * h > size || 14 + hdr + ncol * 4 > size) return FALSE;
    Create(w, h);
    for (int y = 0; y < h; y++) {
        const unsigned char* row = d + bits + stride * (topdown ? y : h - 1 - y);
        uint32_t* out = m_px.data() + (size_t)y * w;
        for (int x = 0; x < w; x++) {
            uint32_t idx;
            switch (bpp) {
            case 1:  idx = (row[x >> 3] >> (7 - (x & 7))) & 1; break;
            case 4:  idx = (row[x >> 1] >> ((x & 1) ? 0 : 4)) & 15; break;
            case 8:  idx = row[x]; break;
            case 24: out[x] = 0xFF000000u | (row[x * 3 + 2] << 16) | (row[x * 3 + 1] << 8) | row[x * 3]; continue;
            default: out[x] = 0xFF000000u | (row[x * 4 + 2] << 16) | (row[x * 4 + 1] << 8) | row[x * 4]; continue;
            }
            const unsigned char* c = pal + 4 * (idx < ncol ? idx : 0);
            out[x] = 0xFF000000u | (c[2] << 16) | (c[1] << 8) | c[0];
        }
    }
    return TRUE;
}

void CDC::Fill(int l, int t, int r, int b, uint32_t c) {
    if (!m_bitmap || !m_bitmap->Bits()) return;
    if (l < 0) l = 0;
    if (t < 0) t = 0;
    if (r > m_bitmap->Width()) r = m_bitmap->Width();
    if (b > m_bitmap->Height()) b = m_bitmap->Height();
    for (int y = t; y < b; y++) {
        uint32_t* p = m_bitmap->Bits() + (size_t)y * m_bitmap->Width();
        for (int x = l; x < r; x++) p[x] = c;
    }
}

// Bresenham, the end point is not drawn (GDI)
BOOL CDC::LineTo(int x1, int y1) {
    uint32_t c = RmtPixel(m_pen ? m_pen->GetColor() : 0);
    int x = m_curX, y = m_curY;
    int dx = abs(x1 - x), dy = -abs(y1 - y);
    int sx = x < x1 ? 1 : -1, sy = y < y1 ? 1 : -1, err = dx + dy;
    while (x != x1 || y != y1) {
        Plot(x, y, c);
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x += sx; }
        if (e2 <= dx) { err += dx; y += sy; }
    }
    m_curX = x1;
    m_curY = y1;
    return TRUE;
}

BOOL CDC::BitBlt(int x, int y, int w, int h, CDC* src, int xs, int ys, DWORD) {
    CBitmap* sb = src ? src->m_bitmap : nullptr;
    if (!m_bitmap || !m_bitmap->Bits() || !sb || !sb->Bits()) return FALSE;
    // clip the rectangle once against both bitmaps, then copy whole rows
    int i0 = std::max({ 0, -x, -xs });
    int i1 = std::min({ w, m_bitmap->Width() - x, sb->Width() - xs });
    int j0 = std::max({ 0, -y, -ys });
    int j1 = std::min({ h, m_bitmap->Height() - y, sb->Height() - ys });
    if (i0 >= i1 || j0 >= j1) return TRUE;
    for (int j = j0; j < j1; j++) {
        uint32_t* dp = m_bitmap->Bits() + (size_t)(y + j) * m_bitmap->Width() + x + i0;
        const uint32_t* sp = sb->Bits() + (size_t)(ys + j) * sb->Width() + xs + i0;
        std::memmove(dp, sp, (size_t)(i1 - i0) * sizeof(uint32_t));
    }
    return TRUE;
}

// nearest neighbour
BOOL CDC::StretchBlt(int x, int y, int w, int h, CDC* src, int xs, int ys, int ws, int hs, DWORD rop) {
    CBitmap* sb = src ? src->m_bitmap : nullptr;
    if (w == ws && h == hs) return BitBlt(x, y, w, h, src, xs, ys, rop);
    if (!m_bitmap || !m_bitmap->Bits() || !sb || !sb->Bits() || w <= 0 || h <= 0) return FALSE;
    // source column of each destination column, -1 when clipped
    std::vector<int> cols(w);
    for (int i = 0; i < w; i++) {
        int dx = x + i, sx = xs + (int)((long long)i * ws / w);
        cols[i] = dx >= 0 && dx < m_bitmap->Width() && sx >= 0 && sx < sb->Width() ? sx : -1;
    }
    for (int j = 0; j < h; j++) {
        int dy = y + j, sy = ys + (int)((long long)j * hs / h);
        if (dy < 0 || dy >= m_bitmap->Height() || sy < 0 || sy >= sb->Height()) continue;
        uint32_t* dp = m_bitmap->Bits() + (size_t)dy * m_bitmap->Width() + x;
        const uint32_t* sp = sb->Bits() + (size_t)sy * sb->Width();
        for (int i = 0; i < w; i++)
            if (cols[i] >= 0) dp[i] = sp[cols[i]];
    }
    return TRUE;
}

#include <thread>
#include <chrono>

void Sleep(DWORD ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }
