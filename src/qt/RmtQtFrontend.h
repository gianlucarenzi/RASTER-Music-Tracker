// RmtQtFrontend.h - Qt5 frontend of RASTER Music Tracker
//
// The tracker GUI is the MFC one (CRmtView, CMainFrame, CRmtDoc) compiled
// against the MfcTypes.h shim: RmtQtBridge (RmtQtFrontend.cpp) implements
// IRmtHost for it and owns the MFC objects, RmtViewWidget shows the view and
// passes it keyboard, mouse and paint events, RmtMainWindow is the frame.
// Only Qt types here: the MFC shim stays out of the moc'ed header.

#pragma once

#include <QMainWindow>
#include <QWidget>

#include <memory>

class RmtQtBridge;

class RmtViewWidget : public QWidget {
    Q_OBJECT
public:
    RmtViewWidget(RmtQtBridge* bridge, QWidget* parent);

protected:
    void paintEvent(QPaintEvent* e) override;
    void keyPressEvent(QKeyEvent* e) override;
    void keyReleaseEvent(QKeyEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseDoubleClickEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void wheelEvent(QWheelEvent* e) override;
    void focusInEvent(QFocusEvent* e) override;
    void focusOutEvent(QFocusEvent* e) override;
    bool focusNextPrevChild(bool) override { return false; }  // Tab belongs to the tracker

private:
    RmtQtBridge* m_bridge;
};

class RmtMainWindow : public QMainWindow {
    Q_OBJECT
public:
    RmtMainWindow();
    ~RmtMainWindow() override;

    // CView::OnInitialUpdate() and the file given on the command line
    void Start(const QString& songFile);

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    std::unique_ptr<RmtQtBridge> m_bridge;
};
