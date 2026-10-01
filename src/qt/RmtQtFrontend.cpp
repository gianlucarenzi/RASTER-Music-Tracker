// RmtQtFrontend.cpp - Qt6 frontend of RASTER Music Tracker (see RmtQtFrontend.h)

#include "StdAfx.h"
#include "resource.h"
#include "RmtDoc.h"
#include "RmtView.h"
#include "MainFrm.h"
#include "Global.h"
#include "GuiHelpers.h"
#include "Song.h"

#include "RmtQtFrontend.h"
#include "ScriptMessages.h"
#include "RmtQtKeys.h"
#include "RmtQtDialogs.h"

#include <QAction>
#include <QCloseEvent>
#include <QComboBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGuiApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QStatusBar>
#include <QToolBar>
#include <QTimer>
#include <QUrl>
#include <QWheelEvent>

#include <map>
#include <set>
#include <vector>

extern CStatusBar* g_statusBar;
extern CSong g_Song;

// ---------------------------------------------------------------------------
// The MFC classes, with their protected constructors and handlers opened up
// for the frontend
// ---------------------------------------------------------------------------

class QtRmtDoc : public CRmtDoc {
public:
    QtRmtDoc() {}
};

class QtMainFrame : public CMainFrame {
public:
    QtMainFrame() {}
};

class QtRmtView : public CRmtView {
public:
    QtRmtView() {}
    using CRmtView::OnTimer;
    using CRmtView::OnDestroy;
    using CRmtView::OnKeyDown;
    using CRmtView::OnKeyUp;
    using CRmtView::OnSysChar;
    using CRmtView::OnLButtonDown;
    using CRmtView::OnLButtonUp;
    using CRmtView::OnLButtonDblClk;
    using CRmtView::OnRButtonDown;
    using CRmtView::OnRButtonUp;
    using CRmtView::OnRButtonDblClk;
    using CRmtView::OnMouseMove;
    using CRmtView::OnMouseWheel;
    using CRmtView::OnSetFocus;
    using CRmtView::OnKillFocus;
};

// ---------------------------------------------------------------------------
// QtCCmdUI - bridges CCmdUI to QAction for ON_UPDATE_COMMAND_UI handlers
// ---------------------------------------------------------------------------

class QtCCmdUI : public CCmdUI {
public:
    QAction* m_action;
    QtCCmdUI(UINT id, QAction* action)
    {
        m_nID = id;
        m_action = action;
    }
    void Enable(BOOL on) override { m_action->setEnabled(on != FALSE); }
    void SetCheck(int check) override
    {
        // setCheckable(true) is set at construction for known toggle items;
        // calling it here during aboutToShow emits QAction::changed → menu repaint.
        if (m_action->isCheckable())
            m_action->setChecked(check != 0);
    }
    void SetRadio(BOOL on) override
    {
        if (m_action->isCheckable())
            m_action->setChecked(on != FALSE);
    }
    void SetText(LPCTSTR text) override
    {
        CCmdUI::SetText(text);
        if (text) m_action->setText(text);
    }
};

// A toolbar button: it becomes a toggle when its handler checks it (no open
// menu to repaint here), and keeps its icon whatever text the handler sets
class QtToolCmdUI : public QtCCmdUI {
public:
    using QtCCmdUI::QtCCmdUI;
    void SetCheck(int check) override
    {
        m_action->setCheckable(true);
        m_action->setChecked(check != 0);
    }
    void SetRadio(BOOL on) override { SetCheck(on); }
    void SetText(LPCTSTR text) override { CCmdUI::SetText(text); }
};

// ---------------------------------------------------------------------------
// RmtQtBridge - IRmtHost for the MFC code, owner of the MFC objects
// ---------------------------------------------------------------------------

class RmtQtBridge : public IRmtHost {
public:
    RmtQtBridge(RmtMainWindow* win) : m_win(win) {}

    RmtMainWindow* m_win;
    RmtViewWidget* m_widget = nullptr;
    QtRmtDoc m_doc;
    QtMainFrame m_frame;
    QtRmtView m_view;
    CBitmap m_windowBitmap; // 1x1, only for GetDC() (CreateCompatibleDC/Bitmap)
    CDC m_windowDC;
    QSize m_widgetSize; // a change forces a redraw (the view resizes its bitmap)
    std::map<UINT_PTR, QTimer*> m_timers;
    std::set<unsigned> m_keysDown;
    // All (id → action) pairs registered in the menu bar, for update-UI polling
    std::vector<std::pair<UINT, QAction*>> m_menuActions;
    bool m_started = false;
    // The toolbars of CMainFrame::OnCreate() (m_wndToolBar, m_ToolBarBlock)
    QToolBar* m_mainToolBar = nullptr;
    QToolBar* m_blockToolBar = nullptr;
    QComboBox* m_linesAfter = nullptr; // m_comboSkipLinesAfterNoteInsert
    struct ToolAction {
        UINT id;
        QAction* action;
        QToolBar* bar;
    };
    std::vector<ToolAction> m_toolActions;
    int m_toolScaling = 0; // g_scaling_percentage of the icon size

    void Attach(RmtViewWidget* widget)
    {
        m_widget = widget;
        m_frame.m_hWnd = (HWND)m_win;
        m_view.m_hWnd = (HWND)widget;
        m_view.m_pDocument = &m_doc;
    }

    void EnsureWindowDC()
    {
        if (!m_windowBitmap.Width()) {
            m_windowBitmap.Create(1, 1);
            m_windowDC.CreateCompatibleDC(nullptr);
            m_windowDC.SelectObject(&m_windowBitmap);
        }
        QSize size(std::max(1, m_widget->width()), std::max(1, m_widget->height()));
        if (size != m_widgetSize) {
            m_widgetSize = size;
            SCREENUPDATE;
        }
    }

    void Paint(QPainter& painter)
    {
        EnsureWindowDC();
        // CRmtView::OnDraw() without its final StretchBlt: the widget shows the
        // view's own bitmap (m_mem_dc), which holds the g_width x g_height
        // screen, and QPainter scales it (nearest neighbour, no smoothing).
        // Only redraw when the tracker has flagged it; expose events re-show
        // the last frame.
        if (m_started && g_screenupdate) {
            if (g_view.debugDisplay) m_view.GetFPS();
            m_view.Resize();
            g_Song.RespectBoundaries();
            m_view.DrawAll();
        }
        NO_SCREENUPDATE;
        const CBitmap& bmp = m_view.m_mem_bitmap;
        if (!bmp.Bits()) {
            painter.fillRect(m_widget->rect(), Qt::black);
            return;
        }
        QImage image((const uchar*)bmp.Bits(), bmp.Width(), bmp.Height(), bmp.Width() * 4, QImage::Format_RGB32);
        if (g_width == m_view.m_width && g_height == m_view.m_height)
            painter.drawImage(0, 0, image);
        else
            painter.drawImage(QRect(0, 0, m_view.m_width, m_view.m_height), image, QRect(0, 0, g_width, g_height));
    }

    // WM_COMMAND: the view, then the frame (as the MFC command routing)
    void Dispatch(UINT id)
    {
        if (m_view.OnPokeyCommand(id) || m_view.OnCmdMsg(id) || m_frame.OnCmdMsg(id)) return;
        switch (id) {
            case ID_APP_ABOUT:
            case ID_HELP_ABOUT_APP:
                if (qEnvironmentVariableIsEmpty("RMT_QT_GRAB"))
                    QMessageBox::about(m_win, "About RMT",
                                       QString("%1\n\nQt frontend (Linux/POSIX)").arg(g_app.GetVersionAndBuild().GetString()));
                break;
            case ID_APP_EXIT:
                m_win->close();
                break;
            case ID_HELP_ONLINE_HELP:
            case ID_HELP_HELP_TOPICS:
                g_app.OpenOnlineHelp();
                break;
            default:
                qDebug("RMT: command %u has no handler", id);
        }
    }

    // Build the full menu bar from the MFC .rc menu structure.
    // Each action dispatches via Dispatch(id); ON_UPDATE_COMMAND_UI is run
    // on aboutToShow so enabled/checked states are kept in sync.
    void BuildMenuBar(QMenuBar* bar)
    {
        // Helper: add a single command item to a menu
        auto addItem = [&](QMenu* menu, const char* text, UINT id,
                           const char* shortcut = nullptr) -> QAction* {
            QAction* act = menu->addAction(text);
            act->setShortcutContext(Qt::ApplicationShortcut);
            if (shortcut) act->setShortcut(QKeySequence(shortcut));
            QObject::connect(act, &QAction::triggered, [this, id] { Dispatch(id); });
            m_menuActions.emplace_back(id, act);
            return act;
        };

        // Store the ID in action data so aboutToShow can find it.
        // Used for ON_UPDATE_COMMAND_UI (enable/checked state refresh).
        auto addItemTagged = [&](QMenu* menu, const char* text, UINT id,
                                 const char* shortcut = nullptr) -> QAction* {
            QAction* act = addItem(menu, text, id, shortcut);
            act->setData(id);
            return act;
        };

        // Toggle item: checkable at construction so SetCheck() in aboutToShow
        // does not emit QAction::changed (which would repaint the open menu).
        auto addToggle = [&](QMenu* menu, const char* text, UINT id) -> QAction* {
            QAction* act = addItemTagged(menu, text, id);
            act->setCheckable(true);
            return act;
        };

        // Connect ON_UPDATE_COMMAND_UI for a menu's *direct* children only.
        // Each submenu is responsible for its own items via its own aboutToShow.
        auto connectUpdate = [&](QMenu* menu) {
            QObject::connect(menu, &QMenu::aboutToShow, [this, menu] {
                if (!m_started) return;
                for (QAction* act : menu->actions()) {
                    UINT id = act->data().toUInt();
                    if (!id) continue;
                    QtCCmdUI ui(id, act);
                    if (!m_view.OnUpdatePokeyCommand(&ui) && !m_view.OnUpdateCmdUI(&ui)) m_frame.OnUpdateCmdUI(&ui);
                }
            });
        };

        // ---- File ----
        QMenu* mFile = bar->addMenu("&File");
        addItemTagged(mFile, "Ne&w", ID_FILE_NEW, "Ctrl+W");
        addItemTagged(mFile, "&Load...", ID_FILE_OPEN, "Ctrl+L");
        addItemTagged(mFile, "&Reload", ID_FILE_RELOAD, "Ctrl+R");
        mFile->addSeparator();
        addItemTagged(mFile, "&Save", ID_FILE_SAVE, "Ctrl+S");
        addItemTagged(mFile, "Save &As...", ID_FILE_SAVE_AS);
        mFile->addSeparator();
        addItemTagged(mFile, "&Import...", ID_FILE_IMPORT);
        addItemTagged(mFile, "&Export As...", ID_FILE_EXPORT_AS);
        mFile->addSeparator();
        addItemTagged(mFile, "E&xit", ID_FILE_EXIT, "Alt+F4");

        // ---- Edit ----
        QMenu* mEdit = bar->addMenu("&Edit");
        addItemTagged(mEdit, "&Undo", ID_UNDO_UNDO, "Ctrl+Z");
        addItemTagged(mEdit, "&Redo", ID_UNDO_REDO, "Ctrl+Y");
        mEdit->addSeparator();
        addItemTagged(mEdit, "&Clear Undo && Redo history", ID_UNDO_CLEARUNDOREDO);

        // ---- Track ----
        QMenu* mTrack = bar->addMenu("&Track");
        addItemTagged(mTrack, "&Copy", ID_TRACK_COPY);
        addItemTagged(mTrack, "&Paste", ID_TRACK_PASTE);
        addItemTagged(mTrack, "Cu&t", ID_TRACK_CUT);
        addItemTagged(mTrack, "&Delete", ID_TRACK_DELETE);
        mTrack->addSeparator();
        addItemTagged(mTrack, "&Info about current track...", ID_TRACK_INFOABOUTUSINGOFACTUALTRACK);
        addItemTagged(mTrack, "Search and &build wise loop", ID_TRACK_SEARCHANDBUILDLOOP);
        addItemTagged(mTrack, "E&xpand loop", ID_TRACK_EXPANDLOOP);
        mTrack->addSeparator();
        addItemTagged(mTrack, "Search and rebuild wise loops in all tracks...", ID_SONG_SEARCHANDBUILDLOOPSINALLTRACKS);
        addItemTagged(mTrack, "Expand loops in all tracks", ID_SONG_EXPANDLOOPSINALLTRACKS);
        addItemTagged(mTrack, "Renumber all tracks...", ID_TRACK_RENUMBERALLTRACKS);
        mTrack->addSeparator();
        addItemTagged(mTrack, "&Load track from file...", ID_TRACK_LOAD);
        addItemTagged(mTrack, "&Save track as...", ID_TRACK_SAVE);
        mTrack->addSeparator();
        addItemTagged(mTrack, "Clear all duplicated tracks, adjust song...", ID_TRACK_CLEARALLDUPLICATEDTRACKS);
        addItemTagged(mTrack, "Clear all tracks unused in song...", ID_TRACK_CLEARALLTRACKSUNUSEDINSONG);
        addItemTagged(mTrack, "All tracks cleanup...", ID_TRACK_ALLTRACKSCLEANUP);

        // ---- Block ----
        QMenu* mBlock = bar->addMenu("&Block");
        addItemTagged(mBlock, "Restore from &backup", ID_BLOCK_BACKUP, "Ctrl+B");
        mBlock->addSeparator();
        addItemTagged(mBlock, "&Copy", ID_BLOCK_COPY, "Ctrl+C");
        addItemTagged(mBlock, "&Paste", ID_BLOCK_PASTE, "Ctrl+V");
        {
            QMenu* sub = mBlock->addMenu("Paste sp&ecial");
            addItemTagged(sub, "&Merge with current content", ID_BLOCK_PASTESPECIAL_MERGEWITHCURRENTCONTENT, "Ctrl+M");
            addItemTagged(sub, "&Volume values only", ID_BLOCK_PASTESPECIAL_VOLUMEVALUESONLY);
            addItemTagged(sub, "&Speed values only", ID_BLOCK_PASTESPECIAL_SPEEDVALUESONLY);
        }
        addItemTagged(mBlock, "Cu&t", ID_BLOCK_CUT, "Ctrl+X");
        addItemTagged(mBlock, "&Delete", ID_BLOCK_DELETE, "Del");
        addItemTagged(mBlock, "Exchange block and Clipboard", ID_BLOCK_EXCHANGE, "Ctrl+E");
        mBlock->addSeparator();
        addItemTagged(mBlock, "E&ffects/tools...", ID_BLOCK_EFFECT, "Ctrl+F");
        mBlock->addSeparator();
        addItemTagged(mBlock, "Select &all", ID_BLOCK_SELECTALL, "Ctrl+A");

        // ---- Instrument ----
        QMenu* mInstr = bar->addMenu("&Instrument");
        addItemTagged(mInstr, "&Copy", ID_INSTR_COPY);
        addItemTagged(mInstr, "&Paste", ID_INSTR_PASTE);
        {
            QMenu* sub = mInstr->addMenu("Paste sp&ecial");
            addItemTagged(sub, "&Volume envelopes only", ID_INSTRUMENT_PASTESPECIAL_VOLUMELRENVELOPESONLY);
            addItemTagged(sub, "&Envelope parameters only", ID_INSTRUMENT_PASTESPECIAL_ENVELOPEPARAMETERSONLY);
            addItemTagged(sub, "Volume envelopes and Envelope parameters only", ID_INSTRUMENT_PASTESPECIAL_VOLUMEENVANDENVELOPEPARSONLY);
            addItemTagged(sub, "&Insert Volume envelopes and Envelope parameters to cursor position", ID_INSTRUMENT_PASTESPECIAL_INSERTVOLUMEENVSANDENVELOPEPARSTOCURSORPOSITION);
            sub->addSeparator();
            addItemTagged(sub, "Volume &L envelope only", ID_INSTRUMENT_PASTESPECIAL_VOLUMELENVELOPEONLY);
            addItemTagged(sub, "Volume &R envelope only", ID_INSTRUMENT_PASTESPECIAL_VOLUMERENVELOPEONLY);
            addItemTagged(sub, "Volume R to L envelope only", ID_INSTRUMENT_PASTESPECIAL_VOLUMERTOLENVELOPEONLY);
            addItemTagged(sub, "Volume L to R envelope only", ID_INSTRUMENT_PASTESPECIAL_VOLUMELTORENVELOPEONLY);
            sub->addSeparator();
            addItemTagged(sub, "&Table only", ID_INSTRUMENT_PASTESPECIAL_TABLEONLY);
        }
        addItemTagged(mInstr, "Cu&t", ID_INSTR_CUT);
        addItemTagged(mInstr, "&Delete", ID_INSTR_DELETE);
        mInstr->addSeparator();
        addItemTagged(mInstr, "&Info about current instrument...", ID_INSTRUMENT_INFO);
        addItemTagged(mInstr, "Change all the instrument occurences...", ID_INSTRUMENT_CHANGE);
        addItemTagged(mInstr, "Renumber all instruments...", ID_INSTRUMENT_RENUMBERALLINSTRUMENTS);
        mInstr->addSeparator();
        addItemTagged(mInstr, "&Load instrument from file...", ID_INSTR_LOAD);
        addItemTagged(mInstr, "&Save instrument as...", ID_INSTR_SAVE);
        mInstr->addSeparator();
        addItemTagged(mInstr, "Clear all unused instruments...", ID_INSTRUMENT_CLEARALLUNUSEDINSTRUMENTS);
        addItemTagged(mInstr, "All instruments cleanup...", ID_INSTR_ALLINSTRUMENTSCLEANUP);

        // ---- Song ----
        QMenu* mSong = bar->addMenu("&Song");
        addItemTagged(mSong, "&Copy line", ID_SONG_COPYLINE);
        addItemTagged(mSong, "&Paste line", ID_SONG_PASTELINE);
        addItemTagged(mSong, "Cl&ear line", ID_SONG_CLEARLINE);
        mSong->addSeparator();
        addItemTagged(mSong, "Delete c&urrent line", ID_SONG_DELETEACTUALLINE, "Ctrl+U");
        addItemTagged(mSong, "&Insert new empty line", ID_SONG_INSERTNEWEMPTYLINE, "Ctrl+I");
        addItemTagged(mSong, "Insert new line with unused empty tracks", ID_SONG_INSERTNEWLINEWITHUNUSEDTRACKS, "Ctrl+P");
        addItemTagged(mSong, "Insert c&opy or clone of song line(s)...", ID_SONG_INSERTCOPYORCLONEOFSONGLINES, "Ctrl+O");
        addItemTagged(mSong, "Insert &new empty unused track to current song position", ID_SONG_PUTNEWEMPTYUNUSEDTRACK, "Ctrl+N");
        addItemTagged(mSong, "Make a track &duplicate to current song position", ID_SONG_MAKETRACKSDUPLICATE, "Ctrl+D");
        mSong->addSeparator();
        addItemTagged(mSong, "Switch song between 4 or 8 channels...", ID_SONG_SONGSWITCH4_8);
        addItemTagged(mSong, "Song columns' order change/copy/clear...", ID_SONG_TRACKSORDERCHANGE);
        addItemTagged(mSong, "Change maximal length of tracks...", ID_SONG_SONGCHANGEMAXIMALLENGTHOFTRACKS);
        mSong->addSeparator();
        addItemTagged(mSong, "All size optimizations...", ID_SONG_SIZEOPTIMIZATION);

        // ---- Pokey ---- (the explorer items are enabled in the Pokey Explorer mode only;
        // the keys are handled by the view, so no accelerator is attached to the actions)
        QMenu* mPokey = bar->addMenu("Poke&y");
        struct PokeyItem {
            const char* text;
            UINT id;
        };
        auto addPokeyChannel = [&](const char* channel, const char* audf, const char* audc, const PokeyItem(&f)[4], const PokeyItem(&c)[4]) {
            QMenu* mChannel = mPokey->addMenu(channel);
            QMenu* mAudf = mChannel->addMenu(audf);
            for (const auto& item : f) addItemTagged(mAudf, item.text, item.id);
            QMenu* mAudc = mChannel->addMenu(audc);
            for (const auto& item : c) addItemTagged(mAudc, item.text, item.id);
        };
        addPokeyChannel("Channel &1", "AUD&F0", "AUD&C0",
                        { { "&Increase By 0x01\t1", ID_POKEY_AUDF0_INCREASE_BY_01 }, { "I&ncrease By 0x10\tShift+1", ID_POKEY_AUDF0_INCREASE_BY_10 }, { "&Decrease By 0x01\tQ", ID_POKEY_AUDF0_DECREASE_BY_01 }, { "D&ecrease By 0x10\tShift+Q", ID_POKEY_AUDF0_DECREASE_BY_10 } },
                        { { "&Increase By 0x01\t2", ID_POKEY_AUDC0_INCREASE_BY_01 }, { "I&ncrease By 0x10\tShift+2", ID_POKEY_AUDC0_INCREASE_BY_10 }, { "&Decrease By 0x01\tW", ID_POKEY_AUDC0_DECREASE_BY_01 }, { "D&ecrease By 0x10\tShift+W", ID_POKEY_AUDC0_DECREASE_BY_10 } });
        addPokeyChannel("Channel &2", "AUD&F1", "AUD&C1",
                        { { "&Increase By 0x01\t3", ID_POKEY_AUDF1_INCREASE_BY_01 }, { "I&ncrease By 0x10\tShift+3", ID_POKEY_AUDF1_INCREASE_BY_10 }, { "&Decrease By 0x01\tE", ID_POKEY_AUDF1_DECREASE_BY_01 }, { "D&ecrease By 0x10\tShift+E", ID_POKEY_AUDF1_DECREASE_BY_10 } },
                        { { "&Increase By 0x01\t4", ID_POKEY_AUDC1_INCREASE_BY_01 }, { "I&ncrease By 0x10\tShift+4", ID_POKEY_AUDC1_INCREASE_BY_10 }, { "&Decrease By 0x01\tR", ID_POKEY_AUDC1_DECREASE_BY_01 }, { "D&ecrease By 0x10\tShift+R", ID_POKEY_AUDC1_DECREASE_BY_10 } });
        addPokeyChannel("Channel &3", "AUD&F2", "AUD&C2",
                        { { "&Increase By 0x01\t5", ID_POKEY_AUDF2_INCREASE_BY_01 }, { "I&ncrease By 0x10\tShift+5", ID_POKEY_AUDF2_INCREASE_BY_10 }, { "&Decrease By 0x01\tT", ID_POKEY_AUDF2_DECREASE_BY_01 }, { "D&ecrease By 0x10\tShift+T", ID_POKEY_AUDF2_DECREASE_BY_10 } },
                        { { "&Increase By 0x01\t6", ID_POKEY_AUDC2_INCREASE_BY_01 }, { "I&ncrease By 0x10\tShift+6", ID_POKEY_AUDC2_INCREASE_BY_10 }, { "&Decrease By 0x01\tY", ID_POKEY_AUDC2_DECREASE_BY_01 }, { "D&ecrease By 0x10\tShift+Y", ID_POKEY_AUDC2_DECREASE_BY_10 } });
        addPokeyChannel("Channel &4", "AUD&F3", "AUD&C3",
                        { { "&Increase By 0x01\t7", ID_POKEY_AUDF3_INCREASE_BY_01 }, { "I&ncrease By 0x10\tShift+7", ID_POKEY_AUDF3_INCREASE_BY_10 }, { "&Decrease By 0x01\tU", ID_POKEY_AUDF3_DECREASE_BY_01 }, { "D&ecrease By 0x10\tShift+U", ID_POKEY_AUDF3_DECREASE_BY_10 } },
                        { { "&Increase By 0x01\t8", ID_POKEY_AUDC3_INCREASE_BY_01 }, { "I&ncrease By 0x10\tShift+8", ID_POKEY_AUDC3_INCREASE_BY_10 }, { "&Decrease By 0x01\tI", ID_POKEY_AUDC3_DECREASE_BY_01 }, { "D&ecrease By 0x10\tShift+I", ID_POKEY_AUDC3_DECREASE_BY_10 } });
        QMenu* mAudctl = mPokey->addMenu("AUDCTL");
        addItemTagged(mAudctl, "Bit &0 - Change Main Base Clock From 64 KHz To 15 KHz\tC", ID_POKEY_AUDCTL_BIT0);
        addItemTagged(mAudctl, "Bit &1 - High Pass Filter Into Channel 2, Clocked By Channel 4\tG", ID_POKEY_AUDCTL_BIT1);
        addItemTagged(mAudctl, "Bit &2 - High Pass Filter Into Channel 1, Clocked By Channel 3\tF", ID_POKEY_AUDCTL_BIT2);
        addItemTagged(mAudctl, "Bit &3 - Join Channels 3 and 4 (16-bit Frequency)\tK", ID_POKEY_AUDCTL_BIT3);
        addItemTagged(mAudctl, "Bit &4 - Join Channels 1 and 2 (16-Bit Frequency)\tJ", ID_POKEY_AUDCTL_BIT4);
        addItemTagged(mAudctl, "Bit &5 - Clock Channel 3 With 1.79 MHz\tD", ID_POKEY_AUDCTL_BIT5);
        addItemTagged(mAudctl, "Bit &6 - Clock channel 1 with 1.79 MHz\tA", ID_POKEY_AUDCTL_BIT6);
        addItemTagged(mAudctl, "Bit &7 - Change The 17-Bit Poly To 9-Bit Poly (Only For Distortion 0 and 8)\tP", ID_POKEY_AUDCTL_BIT7);
        QMenu* mSkctl = mPokey->addMenu("SKCTL");
        addItemTagged(mSkctl, "&Two Tone Mode\tM", ID_POKEY_SKCTL_TWO_TONE_MODE);
        QMenu* mDebugChannel = mPokey->addMenu("Debug &Channel");
        addItemTagged(mDebugChannel, "Next Channel\tEnter", ID_POKEY_NEXTCHANNEL);
        addItemTagged(mDebugChannel, "Previous Channel\tBackspace", ID_POKEY_PREVIOUSCHANNEL);
        QMenu* mDivisor = mPokey->addMenu("&Divisor");
        addItemTagged(mDivisor, "Increase By 0.1\t+", ID_POKEY_DIVISOR_INCREASE_BY_01);
        addItemTagged(mDivisor, "Increase By 1.0\tShift++", ID_POKEY_DIVISOR_INCREASE_BY_1);
        addItemTagged(mDivisor, "Decrease By 0.1\t-", ID_POKEY_DIVISOR_DECREASE_BY_01);
        addItemTagged(mDivisor, "Decrease By 1.0\tShift+-", ID_POKEY_DIVISOR_DECREASE_BY_1);

        // ---- View ---- (toggle items are checkable at construction)
        QMenu* mView = bar->addMenu("&View");
        addItemTagged(mView, "&Configuration...", ID_VIEW_CONFIGURATION);
        mView->addSeparator();
        addItemTagged(mView, "&Tuning...", ID_VIEW_TUNING);
        mView->addSeparator();
        addToggle(mView, "Main &toolbar", ID_VIEW_TOOLBAR);
        addToggle(mView, "&Block toolbar", ID_VIEW_BLOCKTOOLBAR);
        mView->addSeparator();
        addToggle(mView, "&Status Bar", ID_VIEW_STATUS_BAR);
        mView->addSeparator();
        addToggle(mView, "&Play time counter", ID_VIEW_PLAYTIMECOUNTER);
        addToggle(mView, "&Volume analyzer", ID_VIEW_VOLUMEANALYZER);
        addToggle(mView, "Pokey chip &registers", ID_VIEW_POKEYREGS);
        mView->addSeparator();
        addToggle(mView, "&Instrument active help", ID_VIEW_INSTRUMENTACTIVEHELP);

        // ---- Help ----
        QMenu* mHelp = bar->addMenu("&Help");
        addItemTagged(mHelp, "&Help Topics", ID_HELP_HELP_TOPICS);
        addItemTagged(mHelp, "&Online Help", ID_HELP_ONLINE_HELP, "Shift+F1");
        addItemTagged(mHelp, "&About RASTER Music Tracker", ID_HELP_ABOUT_APP);

        // Wire up ON_UPDATE_COMMAND_UI: each menu updates only its own direct
        // children (submenus update their own items via their own aboutToShow).
        std::function<void(QMenu*)> wireUpdate = [&](QMenu* menu) {
            connectUpdate(menu);
            for (QAction* act : menu->actions())
                if (act->menu()) wireUpdate(act->menu());
        };
        for (QAction* act : bar->actions())
            if (act->menu()) wireUpdate(act->menu());
    }

    // The toolbars of the .rc (IDR_MAINFRAME, IDR_TOOLBARBLOCK): 16x15 images
    // of their bitmap, one per button, the light gray of the bitmap is the
    // background; the tooltips and the status bar texts are the ones of the
    // string table
    void BuildToolBars()
    {
        struct Button {
            UINT id;
            const char* status;
            const char* tip;
        };
        auto build = [&](const char* title, UINT bitmap, std::initializer_list<Button> buttons) {
            auto* bar = new QToolBar(title, m_win);
            bar->setObjectName(title);
            bar->setFloatable(false);
            bar->setContextMenuPolicy(Qt::PreventContextMenu);
            QImage image;
            const unsigned char* data;
            size_t size;
            if (RmtFindResource(bitmap, &data, &size)) image = QImage::fromData(data, (int)size, "BMP");
            image = image.convertToFormat(QImage::Format_ARGB32);
            for (int y = 0; y < image.height(); y++) {
                QRgb* line = (QRgb*)image.scanLine(y);
                for (int x = 0; x < image.width(); x++)
                    if ((line[x] & 0xFFFFFF) == 0xC0C0C0) line[x] = 0;
            }
            int index = 0;
            for (const Button& b : buttons) {
                if (!b.id) { // SEPARATOR
                    bar->addSeparator();
                    continue;
                }
                QImage tile = image.copy(index++ * 16, 0, 16, 15);
                QIcon icon;
                for (int scale = 1; scale <= 3; scale++) // nearest neighbour, like the view
                    icon.addPixmap(QPixmap::fromImage(tile.scaled(16 * scale, 15 * scale)));
                if (b.id == ID_BUTTONCOMBO1) { // the combo takes the place of this button
                    m_linesAfter = new QComboBox(bar);
                    for (int i = 0; i <= 8; i++) m_linesAfter->addItem(QString::number(i));
                    m_linesAfter->setCurrentIndex(g_linesafter);
                    m_linesAfter->setToolTip(b.tip);
                    m_linesAfter->setStatusTip(b.status);
                    m_linesAfter->setFocusPolicy(Qt::ClickFocus);
                    // OnSelChangedComboSkipLinesAfterNoteInsert(), OnRestoreFocusToMainWindow()
                    QObject::connect(m_linesAfter, QOverload<int>::of(&QComboBox::activated), m_win, [this](int i) {
                        g_linesafter = i;
                        m_widget->setFocus();
                    });
                    bar->addWidget(m_linesAfter);
                    continue;
                }
                QAction* act = bar->addAction(icon, b.tip);
                act->setToolTip(b.tip);
                act->setStatusTip(b.status);
                UINT id = b.id;
                QObject::connect(act, &QAction::triggered, m_win, [this, id] {
                    Dispatch(id);
                    UpdateToolBars();
                    m_widget->setFocus();
                });
                m_toolActions.push_back({ id, act, bar });
            }
            m_win->addToolBar(Qt::TopToolBarArea, bar);
            return bar;
        };
        m_mainToolBar = build("Main toolbar", IDR_MAINFRAME, {
                                                                 { ID_FILE_NEW, "Create a new song", "New" },
                                                                 { ID_FILE_OPEN, "Load an existing song", "Load" },
                                                                 { ID_FILE_SAVE, "Save the song", "Save" },
                                                                 { ID_FILE_EXPORT_AS, "Export song to file", "Export" },
                                                                 { 0, nullptr, nullptr },
                                                                 { ID_APP_ABOUT, "Display program information, version number and copyright", "About" },
                                                                 { 0, nullptr, nullptr },
                                                                 { ID_PLAY0, "Play song from bookmark position", "Play from bookmark" },
                                                                 { ID_PLAY1, "Play song from start position", "Play from start" },
                                                                 { ID_PLAY2, "Play song from current position", "Play" },
                                                                 { ID_PLAY3, "Play and loop current tracks pattern", "Loop pattern" },
                                                                 { ID_PLAYSTOP, "Stop playing the song. Mute all sounds.", "Stop" },
                                                                 { 0, nullptr, nullptr },
                                                                 { ID_PLAYFOLLOW, "Follow the currently playing position (turn on/off)", "Follow song" },
                                                                 { 0, nullptr, nullptr },
                                                                 { ID_EM_TRACKS, "Move to track edit view", "Track edit" },
                                                                 { ID_EM_INSTRUMENTS, "Move to instrument edit view", "Instrument edit" },
                                                                 { ID_EM_INFO, "Move cursor to Info edit", "Info edit" },
                                                                 { ID_EM_SONG, "Move cursor to song edit", "Song edit" },
                                                                 { 0, nullptr, nullptr },
                                                                 { ID_PROVEMODE, "Edit/Jam mode toggle", "Jam mode" },
                                                                 { 0, nullptr, nullptr },
                                                                 { ID_MIDIONOFF, "MIDI on/off", "MIDI on/off" },
                                                                 { 0, nullptr, nullptr },
                                                                 { ID_BUTTONCOMBO1, "Insert note spacing", "Insert note spacing" },
                                                             });
        m_blockToolBar = build("Block toolbar", IDR_TOOLBARBLOCK, {
                                                                      { ID_BLOCK_BACKUP, "Restore block from backup", "Restore block" },
                                                                      { 0, nullptr, nullptr },
                                                                      { ID_BLOCK_NOTEUP, "Note transposition up", "Transpose up" },
                                                                      { ID_BLOCK_NOTEDOWN, "Note transposition down", "Transpose down" },
                                                                      { ID_BLOCK_INSTRLEFT, "Instrument number change", "Change instrument" },
                                                                      { ID_BLOCK_INSTRRIGHT, "Instrument number change", "Change instrument" },
                                                                      { ID_BLOCK_VOLUMEUP, "Volume up", "Volume up" },
                                                                      { ID_BLOCK_VOLUMEDOWN, "Volume down", "Volume down" },
                                                                      { ID_BLOCK_EFFECT, "Effects/tools", "Effects/tools" },
                                                                      { 0, nullptr, nullptr },
                                                                      { ID_BLOCK_INSTRALL, "Block modification mode", "Block mode" },
                                                                      { 0, nullptr, nullptr },
                                                                      { ID_BLOCK_PLAY, "Play selected block", "Play block" },
                                                                  });
        UpdateToolBars();
    }

    // ON_UPDATE_COMMAND_UI of the buttons (MFC runs it when idle), the combo
    // follows g_linesafter (Ctrl+numpad +/-, new song), the icons the
    // interface size
    void UpdateToolBars()
    {
        if (!m_started || !m_mainToolBar) return;
        for (const ToolAction& t : m_toolActions) {
            if (t.bar->isHidden()) continue;
            QtToolCmdUI ui(t.id, t.action);
            if (!m_view.OnUpdateCmdUI(&ui)) m_frame.OnUpdateCmdUI(&ui);
        }
        if (m_linesAfter->currentIndex() != g_linesafter && g_linesafter >= 0 && g_linesafter <= 8)
            m_linesAfter->setCurrentIndex(g_linesafter);
        if (m_toolScaling != g_scaling_percentage) {
            m_toolScaling = g_scaling_percentage;
            QSize size(16 * m_toolScaling / 100, 15 * m_toolScaling / 100);
            m_mainToolBar->setIconSize(size);
            m_blockToolBar->setIconSize(size);
        }
    }

    QWidget* ControlBarWidget(const CControlBar* bar)
    {
        if (bar == &m_frame.m_wndToolBar) return m_mainToolBar;
        if (bar == &m_frame.m_ToolBarBlock) return m_blockToolBar;
        if (bar == (const CControlBar*)&m_frame.m_wndStatusBar) return m_win->statusBar();
        return nullptr;
    }

    static UINT MouseFlags(Qt::MouseButtons b, Qt::KeyboardModifiers m)
    {
        UINT f = 0;
        if (b & Qt::LeftButton) f |= MK_LBUTTON;
        if (b & Qt::RightButton) f |= MK_RBUTTON;
        if (b & Qt::MiddleButton) f |= MK_MBUTTON;
        if (m & Qt::ShiftModifier) f |= MK_SHIFT;
        if (m & Qt::ControlModifier) f |= MK_CONTROL;
        return f;
    }

    // --- IRmtHost ---

    CFrameWnd* GetMainWnd() override { return &m_frame; }

    void GetClientRect(const CWnd* wnd, RECT* r) override
    {
        QWidget* w = wnd == &m_frame ? (QWidget*)m_win : (QWidget*)m_widget;
        *r = RECT{ 0, 0, w->width(), w->height() };
    }

    void Invalidate(CWnd*) override { m_widget->update(); }

    CDC* GetDC(CWnd*) override
    {
        EnsureWindowDC();
        return &m_windowDC;
    }

    UINT_PTR SetTimer(CWnd*, UINT_PTR id, UINT ms) override
    {
        QTimer*& t = m_timers[id];
        if (!t) {
            t = new QTimer(m_widget);
            QObject::connect(t, &QTimer::timeout, [this, id] { m_view.OnTimer(id); });
        }
        t->start(ms);
        return id;
    }

    BOOL KillTimer(CWnd*, UINT_PTR id) override
    {
        auto it = m_timers.find(id);
        if (it == m_timers.end() || !it->second) return FALSE;
        it->second->stop();
        return TRUE;
    }

    void PostCommand(UINT id) override
    {
        QTimer::singleShot(0, m_widget, [this, id] { Dispatch(id); });
    }

    void Close() override
    {
        QTimer::singleShot(0, m_win, [this] { m_win->close(); });
    }

    int MessageBox(const char* text, const char* caption, UINT type) override
    {
        int scriptAnswer = IDOK;
        if (ScriptMessageBox(text, caption, type, scriptAnswer)) { // rmt /SCRIPT: the console takes the box's place
            return scriptAnswer;
        }
        if (!qEnvironmentVariableIsEmpty("RMT_QT_GRAB")) { // test run: no modal dialogs
            qWarning("[MessageBox] %s: %s", caption ? caption : "", text ? text : "");
            return (type & 0x0F) == MB_YESNO || (type & 0x0F) == MB_YESNOCANCEL ? IDNO : IDOK;
        }
        QMessageBox box(m_win);
        box.setWindowTitle(caption ? caption : "RMT");
        box.setText(text ? text : "");
        switch (type & 0xF0) {
            case MB_ICONERROR: box.setIcon(QMessageBox::Critical); break;
            case MB_ICONWARNING: box.setIcon(QMessageBox::Warning); break;
            case MB_ICONQUESTION: box.setIcon(QMessageBox::Question); break;
            case MB_ICONINFORMATION: box.setIcon(QMessageBox::Information); break;
        }
        switch (type & 0x0F) {
            case MB_OKCANCEL: box.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel); break;
            case MB_YESNO: box.setStandardButtons(QMessageBox::Yes | QMessageBox::No); break;
            case MB_YESNOCANCEL: box.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel); break;
            default: box.setStandardButtons(QMessageBox::Ok);
        }
        switch (box.exec()) {
            case QMessageBox::Yes: return IDYES;
            case QMessageBox::No: return IDNO;
            case QMessageBox::Cancel: return IDCANCEL;
            default: return IDOK;
        }
    }

    HCURSOR LoadCursor(UINT id) override { return (HCURSOR)(uintptr_t)id; }

    void SetCursor(HCURSOR cursor) override
    {
        switch ((UINT)(uintptr_t)cursor) {
            case IDC_CURSORGOTO:
            case IDC_CURSORDLG:
            case IDC_CURSORCHANNELONOFF: m_widget->setCursor(Qt::PointingHandCursor); break;
            case IDC_CURSORENVVOLUME: m_widget->setCursor(Qt::SizeVerCursor); break;
            case IDC_CURSORSETPOS: m_widget->setCursor(Qt::CrossCursor); break;
            case 32514: m_widget->setCursor(Qt::WaitCursor); break; // IDC_WAIT
            default: m_widget->setCursor(Qt::ArrowCursor);
        }
    }

    short GetKeyState(int vk) override
    {
        Qt::KeyboardModifiers m = QGuiApplication::queryKeyboardModifiers();
        bool down;
        switch (vk) {
            case VK_SHIFT:
            case VK_LSHIFT:
            case VK_RSHIFT: down = m & Qt::ShiftModifier; break;
            case VK_CONTROL:
            case VK_LCONTROL:
            case VK_RCONTROL: down = m & Qt::ControlModifier; break;
            case VK_MENU:
            case VK_LMENU:
            case VK_RMENU: down = m & Qt::AltModifier; break;
            case VK_CAPITAL: return 0; // toggle state unknown
            default: down = m_keysDown.count(vk) > 0;
        }
        return down ? (short)0x8000 : 0;
    }

    UINT MapVirtualKeyToChar(UINT vk) override { return RmtVirtualKeyToChar(vk); }

    void SetWindowText(CWnd* wnd, const char* text) override
    {
        if (wnd == &m_frame)
            m_win->setWindowTitle(text);
        else if (wnd == &m_frame.m_wndStatusBar)
            m_win->statusBar()->showMessage(text);
    }

    void ShowControlBar(CControlBar* bar, BOOL show) override
    {
        if (QWidget* w = ControlBarWidget(bar)) w->setVisible(show);
    }
    BOOL IsControlBarVisible(const CControlBar* bar) override
    {
        QWidget* w = ControlBarWidget(bar);
        return w && !w->isHidden();
    }
    void SetStatusText(int, const char* text) override { m_win->statusBar()->showMessage(text); }

    bool FileDialog(bool open, const char* title, const char* initialDir, const char* fileName,
                    const char* filter, DWORD flags, int& filterIndex, CString& path) override
    {
        // MFC filter "Name (*.a)|*.a;*.b|...||" -> Qt name filter "Name (*.a *.A *.b *.B)":
        // the patterns come from the second field, in both cases (Linux file
        // names are case sensitive, Atari files are often upper case)
        QStringList filters, suffixes;
        QList<QStringList> patternLists;
        QStringList parts = QString::fromLocal8Bit(filter ? filter : "").split('|');
        for (int i = 0; i + 1 < parts.size() && !parts[i].isEmpty(); i += 2) {
            QString name = parts[i];
            int paren = name.lastIndexOf('(');
            if (paren > 0) name = name.left(paren).trimmed();
            QStringList patterns;
            for (const QString& p : parts[i + 1].split(';', Qt::SkipEmptyParts)) {
                patterns << p.trimmed().toLower();
                if (p.trimmed().toUpper() != patterns.last()) patterns << p.trimmed().toUpper();
            }
            filters << QString("%1 (%2)").arg(name, patterns.join(' '));
            suffixes << (patterns.value(0).startsWith("*.") ? patterns.value(0).mid(2) : QString());
            patternLists << patterns;
        }
        int index = filterIndex >= 1 && filterIndex <= filters.size() ? filterIndex : 1;

        if (!qEnvironmentVariableIsEmpty("RMT_QT_GRAB")) { // test run: no modal dialogs
            // RMT_QT_FILEDIALOG="a.rmt,b.txt" answers the file dialogs in turn,
            // with the filter that matches the file, or the one given as
            // "file@N" (1-based); none left: cancel
            static QStringList answers = qEnvironmentVariable("RMT_QT_FILEDIALOG").split(',', Qt::SkipEmptyParts);
            if (answers.isEmpty()) {
                qWarning("[FileDialog] %s: cancelled", title ? title : "");
                return false;
            }
            QString answer = answers.takeFirst();
            int at = answer.lastIndexOf('@');
            bool forced = false;
            if (at > 0) {
                int n = answer.mid(at + 1).toInt(&forced);
                if (forced && n >= 1 && n <= filters.size()) {
                    index = n;
                    answer = answer.left(at);
                } else
                    forced = false;
            }
            for (int i = 0; !forced && i < patternLists.size(); i++)
                if (QDir::match(patternLists[i].join(' '), QFileInfo(answer).fileName())) {
                    index = i + 1;
                    break;
                }
            qWarning("[FileDialog] %s: %s (filter %d)", title ? title : "", qPrintable(answer), index);
            path = answer.toLocal8Bit().constData();
            filterIndex = index;
            return true;
        }

        QFileDialog dlg(m_win, title ? QString::fromLocal8Bit(title) : QString());
        dlg.setAcceptMode(open ? QFileDialog::AcceptOpen : QFileDialog::AcceptSave);
        dlg.setFileMode(open ? QFileDialog::ExistingFile : QFileDialog::AnyFile);
        if (!open && !(flags & OFN_OVERWRITEPROMPT)) dlg.setOption(QFileDialog::DontConfirmOverwrite);
        if (initialDir && *initialDir) dlg.setDirectory(QString::fromLocal8Bit(initialDir));
        if (!filters.isEmpty()) {
            dlg.setNameFilters(filters);
            dlg.selectNameFilter(filters[index - 1]);
        }
        // A save without extension gets the one of the chosen filter, so the
        // overwrite prompt checks the file that is really written
        auto setSuffix = [&](int i) { if (!open && i >= 0 && i < suffixes.size()) dlg.setDefaultSuffix(suffixes[i]); };
        setSuffix(index - 1);
        QObject::connect(&dlg, &QFileDialog::filterSelected, [&](const QString& f) { setSuffix(filters.indexOf(f)); });
        if (fileName && *fileName) dlg.selectFile(QString::fromLocal8Bit(fileName));

        bool ok = dlg.exec() == QDialog::Accepted && !dlg.selectedFiles().isEmpty();
        m_widget->setFocus();
        if (!ok) return false;
        path = dlg.selectedFiles().first().toLocal8Bit().constData();
        int selected = filters.indexOf(dlg.selectedNameFilter());
        filterIndex = selected >= 0 ? selected + 1 : index;
        return true;
    }

    INT_PTR DoModal(CDialog* dlg) override
    {
        INT_PTR result = RmtQtRunDialog(m_win, dlg);
        m_widget->setFocus();
        return result;
    }
};

// CRmtApp::OpenOnlineHelp() (Rmt.cpp is MFC only)
void CRmtApp::OpenOnlineHelp()
{
    QDesktopServices::openUrl(QUrl("https://html-preview.github.io/?url=https://github.com/raster-atari-org/RASTER-Music-Tracker/blob/1.35/doc//rmt_en.html"));
}

// ---------------------------------------------------------------------------
// RmtViewWidget
// ---------------------------------------------------------------------------

RmtViewWidget::RmtViewWidget(RmtQtBridge* bridge, QWidget* parent) : QWidget(parent), m_bridge(bridge)
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setMinimumSize(640, 400);
}

void RmtViewWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    m_bridge->Paint(painter);
}

// nFlags of WM_KEYDOWN / WM_KEYUP: scan code, bit 14 = key was already down
static UINT KeyFlags(QKeyEvent* e, bool wasDown)
{
    return (e->nativeScanCode() & 0xFF) | (wasDown ? 0x4000 : 0);
}

void RmtViewWidget::keyPressEvent(QKeyEvent* e)
{
    unsigned vk = RmtVirtualKey(e);
    if (!vk || !m_bridge->m_started) return;
    bool wasDown = m_bridge->m_keysDown.count(vk) > 0;
    m_bridge->m_keysDown.insert(vk);
    // Alt+key is WM_SYSKEYDOWN on Windows: not an OnKeyDown of the view
    if ((e->modifiers() & Qt::AltModifier) && !(e->modifiers() & Qt::ControlModifier)) {
        if (!e->text().isEmpty()) m_bridge->m_view.OnSysChar(e->text().at(0).unicode(), 1, KeyFlags(e, wasDown));
        return;
    }
    m_bridge->m_view.OnKeyDown(vk, 1, KeyFlags(e, wasDown));
}

void RmtViewWidget::keyReleaseEvent(QKeyEvent* e)
{
    if (e->isAutoRepeat()) return;
    unsigned vk = RmtVirtualKey(e);
    if (!vk || !m_bridge->m_started) return;
    m_bridge->m_keysDown.erase(vk);
    m_bridge->m_view.OnKeyUp(vk, 1, KeyFlags(e, true) | 0x8000);
}

void RmtViewWidget::mousePressEvent(QMouseEvent* e)
{
    if (!m_bridge->m_started) return;
    UINT f = RmtQtBridge::MouseFlags(e->buttons(), e->modifiers());
    CPoint p(e->pos().x(), e->pos().y());
    if (e->button() == Qt::LeftButton)
        m_bridge->m_view.OnLButtonDown(f, p);
    else if (e->button() == Qt::RightButton)
        m_bridge->m_view.OnRButtonDown(f, p);
}

void RmtViewWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_bridge->m_started) return;
    UINT f = RmtQtBridge::MouseFlags(e->buttons(), e->modifiers());
    CPoint p(e->pos().x(), e->pos().y());
    if (e->button() == Qt::LeftButton)
        m_bridge->m_view.OnLButtonUp(f, p);
    else if (e->button() == Qt::RightButton)
        m_bridge->m_view.OnRButtonUp(f, p);
}

void RmtViewWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!m_bridge->m_started) return;
    UINT f = RmtQtBridge::MouseFlags(e->buttons(), e->modifiers());
    CPoint p(e->pos().x(), e->pos().y());
    if (e->button() == Qt::LeftButton)
        m_bridge->m_view.OnLButtonDblClk(f, p);
    else if (e->button() == Qt::RightButton)
        m_bridge->m_view.OnRButtonDblClk(f, p);
}

void RmtViewWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!m_bridge->m_started) return;
    m_bridge->m_view.OnMouseMove(RmtQtBridge::MouseFlags(e->buttons(), e->modifiers()), CPoint(e->pos().x(), e->pos().y()));
}

void RmtViewWidget::wheelEvent(QWheelEvent* e)
{
    if (!m_bridge->m_started) return;
    // CRmtView::OnMouseWheel() subtracts the window origin (0,0 here): client coordinates
    QPoint p = e->position().toPoint();
    m_bridge->m_view.OnMouseWheel(RmtQtBridge::MouseFlags(e->buttons(), e->modifiers()),
                                  (short)e->angleDelta().y(), CPoint(p.x(), p.y()));
}

void RmtViewWidget::focusInEvent(QFocusEvent*)
{
    if (m_bridge->m_started) m_bridge->m_view.OnSetFocus(nullptr);
}

void RmtViewWidget::focusOutEvent(QFocusEvent*)
{
    m_bridge->m_keysDown.clear();
    if (m_bridge->m_started) m_bridge->m_view.OnKillFocus(nullptr);
}

// ---------------------------------------------------------------------------
// RmtMainWindow
// ---------------------------------------------------------------------------

RmtMainWindow::RmtMainWindow() : m_bridge(new RmtQtBridge(this))
{
    auto view = new RmtViewWidget(m_bridge.get(), this);
    setCentralWidget(view);
    m_bridge->Attach(view);
    g_rmtHost = m_bridge.get();
    statusBar();
    m_bridge->BuildMenuBar(menuBar());
    m_bridge->BuildToolBars();
    auto* toolUpdate = new QTimer(this);
    QObject::connect(toolUpdate, &QTimer::timeout, this, [this] { m_bridge->UpdateToolBars(); });
    toolUpdate->start(100);
    setWindowTitle("RASTER Music Tracker");
}

RmtMainWindow::~RmtMainWindow()
{
    for (auto& t : m_bridge->m_timers)
        if (t.second) t.second->stop();
    if (m_bridge->m_started) m_bridge->m_view.OnDestroy();
    g_rmtHost = nullptr;
}

void RmtMainWindow::Start(const QString& songFile)
{
    g_statusBar = &m_bridge->m_frame.m_wndStatusBar;
    m_bridge->m_view.OnInitialUpdate();
    m_bridge->m_started = true;
    if (!songFile.isEmpty()) g_Song.FileOpen(songFile.toLocal8Bit().constData(), FALSE);
    centralWidget()->setFocus();
    SCREENUPDATE;
    centralWidget()->update();
}

void RmtMainWindow::closeEvent(QCloseEvent* e)
{
    // CMainFrame::OnClose(): quit only once CRmtView::OnWantExit() said so
    // (unsaved changes, configuration saved), else ask it (File/Exit)
    if (g_closeApplication || !m_bridge->m_started) {
        e->accept();
        return;
    }
    e->ignore();
    m_bridge->Dispatch(ID_FILE_EXIT);
}
