// main-qt.cpp - RASTER Music Tracker, Qt6 frontend
//
// The start-up of CRmtApp::InitInstance() (Rmt.cpp, MFC only), then the
// main window. The first command line argument is a song to open.

#include "StdAfx.h"
#include "Global.h"
#include "Song.h"
#include "Atari.h"
#include "AtariTrackerDriver.h"
#include "Tuning.h"
#include "RmtMidi.h"
#include "resource.h"

#include "RmtQtFrontend.h"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QMenuBar>
#include <QTimer>

extern CSong g_Song;
extern CAtari g_Atari;
extern TTuningSettings g_tuning;
extern TTuningRatios g_tuningRatios;
extern CRmtMidi g_Midi;

int main(int argc, char** argv)
{
    // Qt6 scales by the fractional desktop DPI (Xft.dpi 106 -> 1.1), which
    // blurs the pixel-exact bitmaps; round to whole factors like Qt5 did.
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Round);
#endif
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
    // RMT_QT_KEYS="108,108,106" first presses those keys (Linux evdev codes);
    // RMT_QT_COMMANDS="0xE101,0xE104" then triggers the menu actions of those
    // command IDs, or sends them as WM_COMMAND when they have no menu item
    // (file dialogs are answered by RMT_QT_FILEDIALOG)
    QString grab = qEnvironmentVariable("RMT_QT_GRAB");
    if (!grab.isEmpty()) {
        QString keys = qEnvironmentVariable("RMT_QT_KEYS");
        QString commands = qEnvironmentVariable("RMT_QT_COMMANDS");
        QTimer::singleShot(500, &window, [&window, keys, commands] {
            for (const QString& k : keys.split(',', Qt::SkipEmptyParts)) {
                quint32 scan = k.toUInt() + 8;
                QKeyEvent press(QEvent::KeyPress, 0, Qt::NoModifier, scan, 0, 0);
                QKeyEvent release(QEvent::KeyRelease, 0, Qt::NoModifier, scan, 0, 0);
                QCoreApplication::sendEvent(window.centralWidget(), &press);
                QCoreApplication::sendEvent(window.centralWidget(), &release);
            }
            std::function<QAction*(QMenu*, uint)> findAction = [&](QMenu* m, uint id) -> QAction* {
                for (QAction* a : m->actions()) {
                    if (a->menu()) {
                        if (QAction* f = findAction(a->menu(), id)) return f;
                    } else if (!a->isSeparator() && a->data().toUInt() == id)
                        return a;
                }
                return nullptr;
            };
            for (const QString& c : commands.split(',', Qt::SkipEmptyParts)) {
                uint id = c.trimmed().toUInt(nullptr, 0);
                QAction* action = nullptr;
                for (QAction* top : window.menuBar()->actions())
                    if (top->menu() && (action = findAction(top->menu(), id))) break;
                if (action)
                    action->trigger();
                else
                    g_rmtHost->PostCommand(id); // not in the menu (e.g. accelerators): WM_COMMAND
                QCoreApplication::processEvents();
            }
        });
        int grabMs = qEnvironmentVariableIsSet("RMT_QT_GRAB_MS") ? qEnvironmentVariableIntValue("RMT_QT_GRAB_MS") : 1000;
        QTimer::singleShot(grabMs, &window, [&window, grab] {
            window.grab().save(grab);
            QCoreApplication::exit(0);
        });
    }

    window.Start(argc > 1 ? QString::fromLocal8Bit(argv[1]) : QString());

    // RMT_QT_MENU_TEST: trigger every menu action (except exit) and verify
    // all have registered handlers (no "has no handler" debug warning).
    // Use together with QT_QPA_PLATFORM=offscreen and RMT_QT_GRAB=<any>.
    if (qEnvironmentVariableIsSet("RMT_QT_MENU_TEST")) {
        static QStringList noHandlerMsgs;
        qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& msg) {
            if (msg.contains("has no handler"))
                noHandlerMsgs << msg;
        });

        // IDs that would close or exit the application - skip in tests
        static const QSet<uint> skipIds = {
            (uint)ID_FILE_EXIT,
            (uint)ID_WANTEXIT,
            (uint)ID_APP_EXIT,
        };

        QTimer::singleShot(300, &window, [&window] {
            // Collect all leaf actions (not separators, not submenu headers)
            std::function<QList<QAction*>(QMenu*)> collectLeafs = [&](QMenu* m) -> QList<QAction*> {
                QList<QAction*> result;
                for (QAction* a : m->actions()) {
                    if (a->isSeparator()) continue;
                    if (a->menu())
                        result += collectLeafs(a->menu());
                    else
                        result << a;
                }
                return result;
            };

            QList<QAction*> actions;
            for (QAction* top : window.menuBar()->actions())
                if (top->menu()) actions += collectLeafs(top->menu());

            int triggered = 0;
            for (QAction* a : actions) {
                uint id = a->data().toUInt();
                if (!id || skipIds.contains(id)) continue;
                a->trigger();
                QCoreApplication::processEvents();
                ++triggered;
            }
            fprintf(stdout, "RMT_QT_MENU_TEST: triggered %d menu actions\n", triggered);

            QTimer::singleShot(100, [] {
                if (noHandlerMsgs.isEmpty()) {
                    fprintf(stdout, "RMT_QT_MENU_TEST: PASS\n");
                    QCoreApplication::exit(0);
                } else {
                    fprintf(stdout, "RMT_QT_MENU_TEST: FAIL - missing handlers:\n");
                    for (const QString& m : noHandlerMsgs)
                        fprintf(stdout, "  %s\n", m.toLocal8Bit().constData());
                    QCoreApplication::exit(1);
                }
            });
        });
    }

    int result = app.exec();
    // File/Exit already did this (CRmtView::OnWantExit); any other way out
    // (test hooks, QCoreApplication::exit) must not leave the song timer
    // ticking into the destruction of g_Song, nor MIDI IN calling it
    g_Song.StopTimer();
    g_Midi.MidiOff();
    return result;
}
