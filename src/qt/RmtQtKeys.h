// RmtQtKeys.h - Qt key events -> Win32 virtual-key codes (see RmtQtKeys.cpp)
#pragma once

class QKeyEvent;

// VK_* code of the key of a Qt key event, 0 if unknown
unsigned RmtVirtualKey(const QKeyEvent* e);

// MapVirtualKeyEx(vk, MAPVK_VK_TO_CHAR) for a US layout
unsigned RmtVirtualKeyToChar(unsigned vk);
