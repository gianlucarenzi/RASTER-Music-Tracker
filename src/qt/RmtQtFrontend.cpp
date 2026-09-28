// RmtQtFrontend.cpp - Qt5 frontend of RASTER Music Tracker (see RmtQtFrontend.h)

#include "StdAfx.h"
#include "resource.h"
#include "RmtDoc.h"
#include "RmtView.h"
#include "MainFrm.h"
#include "Global.h"
#include "GuiHelpers.h"
#include "Song.h"

#include "RmtQtFrontend.h"
#include "RmtQtKeys.h"

#include <QCloseEvent>
#include <QDesktopServices>
#include <QGuiApplication>
#include <QImage>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QStatusBar>
#include <QTimer>
#include <QUrl>
#include <QWheelEvent>

#include <map>
#include <set>

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
    CBitmap m_windowBitmap;         // what the view draws into (OnDraw) and the widget shows
    CDC m_windowDC;
    std::map<UINT_PTR, QTimer*> m_timers;
    std::set<unsigned> m_keysDown;
    bool m_started = false;

    void Attach(RmtViewWidget* widget) {
        m_widget = widget;
        m_frame.m_hWnd = (HWND)m_win;
        m_view.m_hWnd = (HWND)widget;
        m_view.m_pDocument = &m_doc;
    }

    void EnsureWindowDC() {
        int w = std::max(1, m_widget->width()), h = std::max(1, m_widget->height());
        if (m_windowBitmap.Width() != w || m_windowBitmap.Height() != h) {
            m_windowBitmap.Create(w, h);
            m_windowDC.CreateCompatibleDC(nullptr);
            m_windowDC.SelectObject(&m_windowBitmap);
            SCREENUPDATE;
        }
    }

    void Paint(QPainter& painter) {
        EnsureWindowDC();
        if (m_started) m_view.OnDraw(&m_windowDC);
        QImage image((const uchar*)m_windowBitmap.Bits(), m_windowBitmap.Width(), m_windowBitmap.Height(),
                     m_windowBitmap.Width() * 4, QImage::Format_RGB32);
        painter.drawImage(0, 0, image);
    }

    // WM_COMMAND: the view, then the frame (as the MFC command routing)
    void Dispatch(UINT id) {
        if (m_view.OnCmdMsg(id) || m_frame.OnCmdMsg(id)) return;
        switch (id) {
        case ID_APP_ABOUT:
            QMessageBox::about(m_win, "About RMT",
                QString("%1\n\nQt frontend (Linux/POSIX)").arg(g_app.GetVersionAndBuild().GetString()));
            break;
        case ID_APP_EXIT:
            m_win->close();
            break;
        default:
            qDebug("RMT: command %u has no handler", id);
        }
    }

    static UINT MouseFlags(Qt::MouseButtons b, Qt::KeyboardModifiers m) {
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

    void GetClientRect(const CWnd* wnd, RECT* r) override {
        QWidget* w = wnd == &m_frame ? (QWidget*)m_win : (QWidget*)m_widget;
        *r = RECT{ 0, 0, w->width(), w->height() };
    }

    void Invalidate(CWnd*) override { m_widget->update(); }

    CDC* GetDC(CWnd*) override { EnsureWindowDC(); return &m_windowDC; }

    UINT_PTR SetTimer(CWnd*, UINT_PTR id, UINT ms) override {
        QTimer*& t = m_timers[id];
        if (!t) {
            t = new QTimer(m_widget);
            QObject::connect(t, &QTimer::timeout, [this, id] { m_view.OnTimer(id); });
        }
        t->start(ms);
        return id;
    }

    BOOL KillTimer(CWnd*, UINT_PTR id) override {
        auto it = m_timers.find(id);
        if (it == m_timers.end() || !it->second) return FALSE;
        it->second->stop();
        return TRUE;
    }

    void PostCommand(UINT id) override {
        QTimer::singleShot(0, m_widget, [this, id] { Dispatch(id); });
    }

    void Close() override {
        QTimer::singleShot(0, m_win, [this] { m_win->close(); });
    }

    int MessageBox(const char* text, const char* caption, UINT type) override {
        if (!qEnvironmentVariableIsEmpty("RMT_QT_GRAB")) {        // test run: no modal dialogs
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

    void SetCursor(HCURSOR cursor) override {
        switch ((UINT)(uintptr_t)cursor) {
        case IDC_CURSORGOTO:
        case IDC_CURSORDLG:
        case IDC_CURSORCHANNELONOFF: m_widget->setCursor(Qt::PointingHandCursor); break;
        case IDC_CURSORENVVOLUME: m_widget->setCursor(Qt::SizeVerCursor); break;
        case IDC_CURSORSETPOS: m_widget->setCursor(Qt::CrossCursor); break;
        case 32514: m_widget->setCursor(Qt::WaitCursor); break;     // IDC_WAIT
        default: m_widget->setCursor(Qt::ArrowCursor);
        }
    }

    short GetKeyState(int vk) override {
        Qt::KeyboardModifiers m = QGuiApplication::queryKeyboardModifiers();
        bool down;
        switch (vk) {
        case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: down = m & Qt::ShiftModifier; break;
        case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: down = m & Qt::ControlModifier; break;
        case VK_MENU: case VK_LMENU: case VK_RMENU: down = m & Qt::AltModifier; break;
        case VK_CAPITAL: return 0;                                  // toggle state unknown
        default: down = m_keysDown.count(vk) > 0;
        }
        return down ? (short)0x8000 : 0;
    }

    UINT MapVirtualKeyToChar(UINT vk) override { return RmtVirtualKeyToChar(vk); }

    void SetWindowText(CWnd* wnd, const char* text) override {
        if (wnd == &m_frame) m_win->setWindowTitle(text);
        else if (wnd == &m_frame.m_wndStatusBar) m_win->statusBar()->showMessage(text);
    }

    void ShowControlBar(CControlBar*, BOOL) override {}            // no toolbars yet
    BOOL IsControlBarVisible(const CControlBar*) override { return FALSE; }
    void SetStatusText(int, const char* text) override { m_win->statusBar()->showMessage(text); }
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
    if (e->button() == Qt::LeftButton) m_bridge->m_view.OnLButtonDown(f, p);
    else if (e->button() == Qt::RightButton) m_bridge->m_view.OnRButtonDown(f, p);
}

void RmtViewWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (!m_bridge->m_started) return;
    UINT f = RmtQtBridge::MouseFlags(e->buttons(), e->modifiers());
    CPoint p(e->pos().x(), e->pos().y());
    if (e->button() == Qt::LeftButton) m_bridge->m_view.OnLButtonUp(f, p);
    else if (e->button() == Qt::RightButton) m_bridge->m_view.OnRButtonUp(f, p);
}

void RmtViewWidget::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (!m_bridge->m_started) return;
    UINT f = RmtQtBridge::MouseFlags(e->buttons(), e->modifiers());
    CPoint p(e->pos().x(), e->pos().y());
    if (e->button() == Qt::LeftButton) m_bridge->m_view.OnLButtonDblClk(f, p);
    else if (e->button() == Qt::RightButton) m_bridge->m_view.OnRButtonDblClk(f, p);
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
    setWindowTitle("RASTER Music Tracker");
}

RmtMainWindow::~RmtMainWindow()
{
    for (auto& t : m_bridge->m_timers) if (t.second) t.second->stop();
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
