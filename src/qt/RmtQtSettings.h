// RmtQtSettings.h - what the Qt frontend keeps of the window (RmtQtSettings.cpp)

#pragma once

#include <QByteArray>

// The size and the position of the main window of the last session (QMainWindow::saveGeometry()), empty
// when there is none yet
QByteArray RmtLoadWindowGeometry();
void RmtSaveWindowGeometry(const QByteArray& geometry);
