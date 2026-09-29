// RmtQtKeys.cpp - Qt key events -> Win32 virtual-key codes
//
// The tracker reads Win32 VK_* codes (OnKeyDown, GetKeyState): the note
// keys are positions on the keyboard (QWERTY rows), like on Windows with a
// US layout. So on Linux the physical key (scan code) is mapped to the VK
// code of the US layout; on Windows Qt gives the native VK code; elsewhere
// the Qt key code is used.

#include "RmtQtKeys.h"

#include <QKeyEvent>

// Win32 VK codes (the same values as MfcTypes.h, repeated to keep this file
// free of the MFC shim)
enum : unsigned {
    K_BACK = 0x08, K_TAB = 0x09, K_RETURN = 0x0D, K_SHIFT = 0x10, K_CONTROL = 0x11, K_MENU = 0x12,
    K_PAUSE = 0x13, K_CAPITAL = 0x14, K_ESCAPE = 0x1B, K_SPACE = 0x20, K_PRIOR = 0x21, K_NEXT = 0x22,
    K_END = 0x23, K_HOME = 0x24, K_LEFT = 0x25, K_UP = 0x26, K_RIGHT = 0x27, K_DOWN = 0x28,
    K_INSERT = 0x2D, K_DELETE = 0x2E, K_NUMPAD0 = 0x60, K_MULTIPLY = 0x6A, K_ADD = 0x6B,
    K_SUBTRACT = 0x6D, K_DECIMAL = 0x6E, K_DIVIDE = 0x6F, K_F1 = 0x70, K_NUMLOCK = 0x90, K_SCROLL = 0x91,
    K_MEDIA_NEXT = 0xB0, K_MEDIA_PREV = 0xB1, K_MEDIA_STOP = 0xB2, K_MEDIA_PLAY = 0xB3,
    K_OEM_1 = 0xBA, K_OEM_PLUS = 0xBB, K_OEM_COMMA = 0xBC, K_OEM_MINUS = 0xBD, K_OEM_PERIOD = 0xBE,
    K_OEM_2 = 0xBF, K_OEM_3 = 0xC0, K_OEM_4 = 0xDB, K_OEM_5 = 0xDC, K_OEM_6 = 0xDD, K_OEM_7 = 0xDE,
    K_OEM_102 = 0xE2,
};

#if defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
// Linux evdev key codes (linux/input-event-codes.h) -> VK, US layout.
// X11 and Wayland report them + 8 as the native scan code.
static unsigned EvdevToVk(unsigned code)
{
    static const unsigned char row1[] = "1234567890";           // 2..11
    static const unsigned char row2[] = "QWERTYUIOP";           // 16..25
    static const unsigned char row3[] = "ASDFGHJKL";            // 30..38
    static const unsigned char row4[] = "ZXCVBNM";              // 44..50
    if (code >= 2 && code <= 11) return row1[code - 2];
    if (code >= 16 && code <= 25) return row2[code - 16];
    if (code >= 30 && code <= 38) return row3[code - 30];
    if (code >= 44 && code <= 50) return row4[code - 44];
    if (code >= 59 && code <= 68) return K_F1 + (code - 59);    // F1..F10
    switch (code) {
    case 1: return K_ESCAPE;
    case 12: return K_OEM_MINUS;
    case 13: return K_OEM_PLUS;
    case 14: return K_BACK;
    case 15: return K_TAB;
    case 26: return K_OEM_4;
    case 27: return K_OEM_6;
    case 28: return K_RETURN;
    case 29: case 97: return K_CONTROL;
    case 39: return K_OEM_1;
    case 40: return K_OEM_7;
    case 41: return K_OEM_3;
    case 42: case 54: return K_SHIFT;
    case 43: return K_OEM_5;
    case 51: return K_OEM_COMMA;
    case 52: return K_OEM_PERIOD;
    case 53: return K_OEM_2;
    case 55: return K_MULTIPLY;
    case 56: case 100: return K_MENU;
    case 57: return K_SPACE;
    case 58: return K_CAPITAL;
    case 69: return K_NUMLOCK;
    case 70: return K_SCROLL;
    case 71: return K_NUMPAD0 + 7;
    case 72: return K_NUMPAD0 + 8;
    case 73: return K_NUMPAD0 + 9;
    case 74: return K_SUBTRACT;
    case 75: return K_NUMPAD0 + 4;
    case 76: return K_NUMPAD0 + 5;
    case 77: return K_NUMPAD0 + 6;
    case 78: return K_ADD;
    case 79: return K_NUMPAD0 + 1;
    case 80: return K_NUMPAD0 + 2;
    case 81: return K_NUMPAD0 + 3;
    case 82: return K_NUMPAD0;
    case 83: return K_DECIMAL;
    case 86: return K_OEM_102;
    case 87: return K_F1 + 10;                                  // F11
    case 88: return K_F1 + 11;                                  // F12
    case 96: return K_RETURN;                                   // keypad Enter
    case 98: return K_DIVIDE;
    case 102: return K_HOME;
    case 103: return K_UP;
    case 104: return K_PRIOR;
    case 105: return K_LEFT;
    case 106: return K_RIGHT;
    case 107: return K_END;
    case 108: return K_DOWN;
    case 109: return K_NEXT;
    case 110: return K_INSERT;
    case 111: return K_DELETE;
    case 119: return K_PAUSE;
    case 163: return K_MEDIA_NEXT;
    case 164: return K_MEDIA_PLAY;
    case 165: return K_MEDIA_PREV;
    case 166: return K_MEDIA_STOP;
    }
    return 0;
}
#endif

// fallback: Qt key code (layout dependent)
static unsigned QtKeyToVk(int key, bool keypad)
{
    if (key >= Qt::Key_A && key <= Qt::Key_Z) return 'A' + (key - Qt::Key_A);
    if (key >= Qt::Key_0 && key <= Qt::Key_9) return keypad ? K_NUMPAD0 + (key - Qt::Key_0) : '0' + (key - Qt::Key_0);
    if (key >= Qt::Key_F1 && key <= Qt::Key_F12) return K_F1 + (key - Qt::Key_F1);
    switch (key) {
    case Qt::Key_Escape: return K_ESCAPE;
    case Qt::Key_Tab: case Qt::Key_Backtab: return K_TAB;
    case Qt::Key_Backspace: return K_BACK;
    case Qt::Key_Return: case Qt::Key_Enter: return K_RETURN;
    case Qt::Key_Insert: return K_INSERT;
    case Qt::Key_Delete: return K_DELETE;
    case Qt::Key_Pause: return K_PAUSE;
    case Qt::Key_Home: return K_HOME;
    case Qt::Key_End: return K_END;
    case Qt::Key_Left: return K_LEFT;
    case Qt::Key_Up: return K_UP;
    case Qt::Key_Right: return K_RIGHT;
    case Qt::Key_Down: return K_DOWN;
    case Qt::Key_PageUp: return K_PRIOR;
    case Qt::Key_PageDown: return K_NEXT;
    case Qt::Key_Shift: return K_SHIFT;
    case Qt::Key_Control: return K_CONTROL;
    case Qt::Key_Alt: return K_MENU;
    case Qt::Key_CapsLock: return K_CAPITAL;
    case Qt::Key_Space: return K_SPACE;
    case Qt::Key_Asterisk: return K_MULTIPLY;
    case Qt::Key_Plus: return keypad ? K_ADD : K_OEM_PLUS;
    case Qt::Key_Minus: return keypad ? K_SUBTRACT : K_OEM_MINUS;
    case Qt::Key_Slash: return keypad ? K_DIVIDE : K_OEM_2;
    case Qt::Key_Period: return keypad ? K_DECIMAL : K_OEM_PERIOD;
    case Qt::Key_Equal: return K_OEM_PLUS;
    case Qt::Key_Comma: return K_OEM_COMMA;
    case Qt::Key_Semicolon: return K_OEM_1;
    case Qt::Key_Apostrophe: return K_OEM_7;
    case Qt::Key_QuoteLeft: return K_OEM_3;
    case Qt::Key_BracketLeft: return K_OEM_4;
    case Qt::Key_Backslash: return K_OEM_5;
    case Qt::Key_BracketRight: return K_OEM_6;
    case Qt::Key_Less: return K_OEM_102;
    case Qt::Key_MediaNext: return K_MEDIA_NEXT;
    case Qt::Key_MediaPrevious: return K_MEDIA_PREV;
    case Qt::Key_MediaPlay: case Qt::Key_MediaTogglePlayPause: return K_MEDIA_PLAY;
    case Qt::Key_MediaStop: return K_MEDIA_STOP;
    }
    return 0;
}

unsigned RmtVirtualKey(const QKeyEvent* e)
{
#if defined(Q_OS_WIN)
    if (e->nativeVirtualKey()) return e->nativeVirtualKey();
#elif defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD)
    if (e->nativeScanCode() > 8) {
        unsigned vk = EvdevToVk(e->nativeScanCode() - 8);
        if (vk) return vk;
    }
#endif
    return QtKeyToVk(e->key(), e->modifiers() & Qt::KeypadModifier);
}

unsigned RmtVirtualKeyToChar(unsigned vk)
{
    // MapVirtualKeyEx(MAPVK_VK_TO_CHAR) with a US layout: unshifted character
    if ((vk >= '0' && vk <= '9') || (vk >= 'A' && vk <= 'Z')) return vk;
    switch (vk) {
    case K_SPACE: return ' ';
    case K_OEM_1: return ';';
    case K_OEM_PLUS: return '=';
    case K_OEM_COMMA: return ',';
    case K_OEM_MINUS: return '-';
    case K_OEM_PERIOD: return '.';
    case K_OEM_2: return '/';
    case K_OEM_3: return '`';
    case K_OEM_4: return '[';
    case K_OEM_5: return '\\';
    case K_OEM_6: return ']';
    case K_OEM_7: return '\'';
    case K_OEM_102: return '\\';
    case K_MULTIPLY: return '*';
    case K_ADD: return '+';
    case K_SUBTRACT: return '-';
    case K_DIVIDE: return '/';
    case K_DECIMAL: return '.';
    }
    if (vk >= K_NUMPAD0 && vk <= K_NUMPAD0 + 9) return '0' + (vk - K_NUMPAD0);
    return 0;
}
