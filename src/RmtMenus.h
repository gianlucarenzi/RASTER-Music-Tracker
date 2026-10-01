#pragma once

#include "PlatformTypes.h"

// The main menu and the accelerator table of Rmt.rc (IDR_MAINFRAME), as data generated at configure time by
// cmake/GenerateRcTables.cmake. The Qt frontend builds its menu bar and its shortcuts from them
// (qt/RmtQtFrontend.cpp); the script command "dump actions" reads the keys from them (ActionTable.cpp).

enum class RmtMenuKind : unsigned char { Popup,
                                         Item,
                                         Separator,
                                         End };

struct TRmtMenuEntry {
    int level; // 0: a menu of the menu bar, 1: its items and submenus, ...
    RmtMenuKind kind;
    const char* text; // "&Save\tCtrl+S" as written in the .rc: mnemonic &, the key hint after the tab
    UINT id;          // 0 for a popup or a separator
};

// The modifiers of an accelerator
enum : UINT { RMT_ACC_CONTROL = 1,
              RMT_ACC_SHIFT = 2,
              RMT_ACC_ALT = 4 };

struct TRmtAccelerator {
    UINT vk;        // a virtual key (VK_F9, 'S')
    UINT modifiers; // RMT_ACC_*
    UINT id;
};

extern const TRmtMenuEntry g_rmtMenu[];           // ends with kind End
extern const TRmtAccelerator g_rmtAccelerators[]; // ends with id 0

// "Ctrl+Shift+S" for an accelerator: the modifiers in MFC's order, English key names (as the other programs name them)
std::string RmtAcceleratorText(const TRmtAccelerator& accelerator);
