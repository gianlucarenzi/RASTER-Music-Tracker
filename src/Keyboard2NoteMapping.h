#pragma once

#include "General.h" // KeyboardLayout

extern char NoteKey(int vk);
extern char NumbKey(int vk);
extern char Numblock09Key(int vk);

// The QWERTY key at the position of vk on the layout's keyboard, for keys that
// mean a position (the Pokey Explorer's): the three letter rows, the ISO key
// discounted; + and - (0xBB/0xBD) and everything else stay themselves.
extern int ToQwertyPosition(int vk, KeyboardLayout layout);
