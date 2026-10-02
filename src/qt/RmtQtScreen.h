// RmtQtScreen.h - the size of the window and the interface size of RITMO from the size of the screen
//
// The tracker draws a pixel-art screen whose layout has a size of its own: the "Interface size" of
// the options (100 to 300 %, SCALEPERCENTAGE) enlarges it. Only whole multiples keep the pixels
// exact, so the automatic choice picks 100, 200 or 300 %. The default window of the program is the
// size of the screen of a small laptop (1366x768), reduced to what the screen has when the screen is
// smaller. Without Qt types of the program, so it can be tried alone.

#pragma once

#include <algorithm>

// The size the tracker needs at 100 % to be used: the stereo song screen with the song area is about
// 1100 pixels wide, and 600 high shows a useful number of lines of the tracks.
constexpr int RMT_MIN_USEFUL_WIDTH = 1100;
constexpr int RMT_MIN_USEFUL_HEIGHT = 600;

// The size of the window the first time the program starts (at 100 %): the one of a 1366x768 laptop
constexpr int RMT_DEFAULT_WINDOW_WIDTH = 1366;
constexpr int RMT_DEFAULT_WINDOW_HEIGHT = 768;

// The room the title bar and the frame of a window take, and the part of the screen the window may use
constexpr int RMT_WINDOW_DECORATION_HEIGHT = 48;

// The interface size (100, 200 or 300) for the available area of a screen, in logical pixels: the
// largest one at which the tracker still has the room it needs, with a margin of 5 %
inline int RmtScalingForScreen(int availableWidth, int availableHeight)
{
    for (int scale = 300; scale > 100; scale -= 100) {
        if (RMT_MIN_USEFUL_WIDTH * scale / 100 <= availableWidth * 95 / 100 &&
            RMT_MIN_USEFUL_HEIGHT * scale / 100 <= availableHeight * 95 / 100) {
            return scale;
        }
    }
    return 100;
}

// The size of a window that is wanted width x height, reduced so that it and its title bar fit in
// the available area of the screen
inline void RmtFitToScreen(int& width, int& height, int availableWidth, int availableHeight)
{
    width = std::min(width, availableWidth);
    height = std::min(height, std::max(availableHeight - RMT_WINDOW_DECORATION_HEIGHT, RMT_MIN_USEFUL_HEIGHT / 2));
}
