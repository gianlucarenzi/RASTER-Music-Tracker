// main-qt.cpp - RASTER Music Tracker, Qt5 frontend
//
// The start-up of CRmtApp::InitInstance() (Rmt.cpp, MFC only), then the
// main window. The first command line argument is a song to open.

#include "StdAfx.h"
#include "Global.h"
#include "Song.h"
#include "Atari.h"
#include "AtariTrackerDriver.h"
#include "Tuning.h"

#include "RmtQtFrontend.h"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QTimer>

extern CSong g_Song;
extern CAtari g_Atari;
extern TTuningSettings g_tuning;
extern TTuningRatios g_tuningRatios;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    QApplication::setApplicationName("RASTER Music Tracker");

    SetProgramFolderPath(CString((QDir::toNativeSeparators(QCoreApplication::applicationDirPath()) + QDir::separator()).toLocal8Bit().constData()));

    // Without the 6502 emulation (sa_c6502.dll on Windows) RMT still edits
    // songs; CRmtApp would exit here, the Qt frontend goes on.
    g_Atari.Init();
    g_tuning.Initialize(g_Song.IsNTSC());
    g_tuningRatios.Initialize();
    g_Atari.Init(g_Song.IsNTSC());

    g_AtariTrackerDriver = new CAtariTrackerDriver(g_Atari);
    g_AtariTrackerDriver->LoadRMTRoutines(g_trackerDriverVersion);
    g_AtariTrackerDriver->Init();

    g_Song.ClearSong(8);

    RmtMainWindow window;
    window.resize(1280, 800);
    window.show();
    // test hook: RMT_QT_GRAB=file.png saves the window after 1 s and quits
    // (with QT_QPA_PLATFORM=offscreen it runs without a display; the delay is
    // RMT_QT_GRAB_MS);
    // RMT_QT_KEYS="108,108,106" first presses those keys (Linux evdev codes)
    QString grab = qEnvironmentVariable("RMT_QT_GRAB");
    if (!grab.isEmpty()) {
        QString keys = qEnvironmentVariable("RMT_QT_KEYS");
        QTimer::singleShot(500, &window, [&window, keys] {
            for (const QString& k : keys.split(',', Qt::SkipEmptyParts)) {
                quint32 scan = k.toUInt() + 8;
                QKeyEvent press(QEvent::KeyPress, 0, Qt::NoModifier, scan, 0, 0);
                QKeyEvent release(QEvent::KeyRelease, 0, Qt::NoModifier, scan, 0, 0);
                QCoreApplication::sendEvent(window.centralWidget(), &press);
                QCoreApplication::sendEvent(window.centralWidget(), &release);
            }
        });
        int grabMs = qEnvironmentVariableIsSet("RMT_QT_GRAB_MS") ? qEnvironmentVariableIntValue("RMT_QT_GRAB_MS") : 1000;
        QTimer::singleShot(grabMs, &window, [&window, grab] {
            window.grab().save(grab);
            QCoreApplication::exit(0);
        });
    }

    window.Start(argc > 1 ? QString::fromLocal8Bit(argv[1]) : QString());
    return app.exec();
}
