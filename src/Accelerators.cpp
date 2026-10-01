#include "PlatformTypes.h"
#include "RmtMenus.h"

#include <string>

// "Ctrl+Shift+S" for an accelerator of Rmt.rc: MFC's order of the modifiers and English key names (a generated
// document must read the same on every system)
std::string RmtAcceleratorText(const TRmtAccelerator& accelerator)
{
    std::string result;
    if (accelerator.modifiers & RMT_ACC_CONTROL) {
        result += "Ctrl+";
    }
    if (accelerator.modifiers & RMT_ACC_SHIFT) {
        result += "Shift+";
    }
    if (accelerator.modifiers & RMT_ACC_ALT) {
        result += "Alt+";
    }
    const UINT key = accelerator.vk;
    if (key >= VK_F1 && key <= VK_F1 + 23) {
        return result + "F" + std::to_string(key - VK_F1 + 1);
    }
    if ((key >= '0' && key <= '9') || (key >= 'A' && key <= 'Z')) {
        return result + (char)key;
    }
    if (key >= VK_NUMPAD0 && key <= VK_NUMPAD0 + 9) {
        return result + "Num " + std::to_string(key - VK_NUMPAD0);
    }
    switch (key) {
        case VK_SPACE: return result + "Space";
        case VK_ESCAPE: return result + "Esc";
        case VK_RETURN: return result + "Enter";
        case VK_TAB: return result + "Tab";
        case VK_BACK: return result + "Backspace";
        case VK_DELETE: return result + "Del";
        case VK_INSERT: return result + "Ins";
        case VK_HOME: return result + "Home";
        case VK_END: return result + "End";
        case VK_PRIOR: return result + "PgUp";
        case VK_NEXT: return result + "PgDn";
        case VK_UP: return result + "Up";
        case VK_DOWN: return result + "Down";
        case VK_LEFT: return result + "Left";
        case VK_RIGHT: return result + "Right";
        case VK_ADD: return result + "Num +";
        case VK_SUBTRACT: return result + "Num -";
        case VK_MULTIPLY: return result + "Num *";
        case VK_DIVIDE: return result + "Num /";
        case VK_DECIMAL: return result + "Num .";
        case VK_PAUSE: return result + "Pause";
        default: break;
    }
    char other[16];
    snprintf(other, sizeof(other), "VK_%02X", key);
    return result + other;
}
