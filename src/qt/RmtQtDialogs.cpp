// RmtQtDialogs.cpp - the MFC dialogs of RMT, rewritten with Qt (see RmtQtDialogs.h)

#include "RmtQtDialogs.h"

#include "resource.h"
#include "Global.h"
#include "TrackTypes.h"
#include "filenewdlg.h"
#include "importdlgs.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStyle>
#include <QTimer>
#include <QVBoxLayout>

#include <functional>

// Run a dialog. Test runs (RMT_QT_GRAB) show no modal dialogs: they are
// cancelled, unless RMT_QT_DIALOG=file.png is set, then the dialog is shown,
// saved and confirmed with ok(), what the user does to confirm it. The first
// dialog is saved to file.png, the next ones to file-2.png, file-3.png...
static bool ExecDialog(QDialog& dialog, const std::function<void()>& ok)
{
    if (!qEnvironmentVariableIsEmpty("RMT_QT_GRAB")) {
        QString shot = qEnvironmentVariable("RMT_QT_DIALOG");
        if (shot.isEmpty()) {
            qWarning("[Dialog] %s: cancelled", qPrintable(dialog.windowTitle()));
            return false;
        }
        static int count = 0;
        if (++count > 1) {
            QFileInfo fi(shot);
            shot = fi.path() + "/" + fi.completeBaseName() + QString("-%1.").arg(count) + fi.suffix();
        }
        qWarning("[Dialog] %s: %s", qPrintable(dialog.windowTitle()), qPrintable(shot));
        QTimer::singleShot(500, &dialog, [&dialog, shot, ok] {
            dialog.grab().save(shot);
            ok();
        });
    }
    return dialog.exec() == QDialog::Accepted;
}

// ---------------------------------------------------------------------------
// IDD_FILENEW - File -> New (CFileNewDlg, filenewdlg.cpp)
// ---------------------------------------------------------------------------

static INT_PTR RunFileNew(QWidget* parent, CFileNewDlg* dlg)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("New RMT module");

    auto* length = new QSpinBox(&dialog);
    length->setRange(1, TRACKLEN);                  // DDV_MinMaxInt(1, 256)
    length->setValue(dlg->m_maxTrackLength);

    auto* type = new QComboBox(&dialog);
    type->addItems({ "MONO - 4 TRACKS", "STEREO - 8 TRACKS" });
    type->setCurrentIndex(dlg->m_comboMonoOrStereo);

    auto* form = new QFormLayout;
    form->addRow("Maximal length of tracks", length);
    form->addRow(type);

    auto* warning = new QLabel("Warning: Current data will be discarded!\nThis operation cannot be undone!", &dialog);
    warning->setAlignment(Qt::AlignCenter);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);

    auto* layout = new QVBoxLayout(&dialog);
    layout->addLayout(form);
    layout->addWidget(warning);
    layout->addWidget(buttons);

    // CFileNewDlg::OnOK(): tracks longer than 64 lines need a confirmation
    auto ok = [&] {
        if (length->value() > 64) {
            int r = MessageBox(g_hwnd, "Warning:\nLength of tracks is greater than 64.\nRMT's internal module format allows for a maximum of\n256 bytes for each track. It is not recommended to use\na large number of events in long tracks.\nEach track event (note or speed command) uses about 2 bytes.\n\nWhen saving the RMT file it will report any problems with it.\n\nOk?", "New RMT module - Warning", MB_YESNO | MB_ICONQUESTION);
            if (r != IDYES) return;
        }
        dialog.accept();
    };
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, ok);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (!ExecDialog(dialog, ok)) return IDCANCEL;
    dlg->m_maxTrackLength = length->value();
    dlg->m_comboMonoOrStereo = type->currentIndex();
    return IDOK;
}

// ---------------------------------------------------------------------------
// IDD_IMPORTMOD / IDD_IMPORTTMC - import options (CImportModDlg,
// CImportTmcDlg, importdlgs.cpp)
// ---------------------------------------------------------------------------

static QCheckBox* AddCheck(QVBoxLayout* layout, const char* text, int indent = 0)
{
    auto* check = new QCheckBox(text);
    check->setChecked(true);                        // OnInitDialog: all options on
    if (indent) {
        auto* row = new QHBoxLayout;
        row->addSpacing(indent);
        row->addWidget(check);
        layout->addLayout(row);
    }
    else
        layout->addWidget(check);
    return check;
}

static QLabel* AddHeading(QVBoxLayout* layout, const char* text)
{
    auto* label = new QLabel(text);
    layout->addSpacing(4);
    layout->addWidget(label);
    return label;
}

static QDialogButtonBox* AddOkCancel(QDialog& dialog, QVBoxLayout* layout)
{
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    QObject::connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    return buttons;
}

static INT_PTR RunImportMod(QWidget* parent, CImportModDlg* dlg)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Import ProTracker Module");
    auto* layout = new QVBoxLayout(&dialog);

    layout->addWidget(new QLabel(QString::fromLocal8Bit(dlg->m_info.GetString())));

    auto* typeBox = new QGroupBox("Type of RMT module");
    auto* typeLayout = new QVBoxLayout(typeBox);
    auto* radio1 = new QRadioButton(QString::fromLocal8Bit(dlg->m_txtradio1.GetString()));
    auto* radio2 = new QRadioButton(QString::fromLocal8Bit(dlg->m_txtradio2.GetString()));
    radio1->setChecked(true);
    typeLayout->addWidget(radio1);
    typeLayout->addWidget(radio2);
    layout->addWidget(typeBox);

    AddHeading(layout, "Note events:");
    auto* check1 = AddCheck(layout, "Shift down octave of all instruments if song tuning is too high and if it is possible.");
    auto* check5 = AddCheck(layout, "Substitute all portamento effects by inserting of calculated notes.");
    AddHeading(layout, "Volume events:");
    auto* check2 = AddCheck(layout, "Increase the volume entries in tracks to spread full volume range.");
    auto* check3 = AddCheck(layout, "Decrease the instruments' volume envelopes in accordance with tracks entries increasing.", 16);
    auto* check4 = AddCheck(layout, "Decrease the instruments' volume envelopes according to sample volume entry.");
    AddHeading(layout, "Special:");
    auto* check8 = AddCheck(layout, "Use the Fourier transformation for detection of samples' tunings.");
    check8->setChecked(false);                      // WS_DISABLED in the resource
    check8->setEnabled(false);
    AddHeading(layout, "Size optimizations:");
    auto* check6 = AddCheck(layout, "Search and build wise loops in tracks.");
    auto* check7 = AddCheck(layout, "Truncate unused parts of tracks (only if it has data saving effect).");

    // CImportModDlg::OnCheck2(): check 3 only applies with check 2
    QObject::connect(check2, &QCheckBox::toggled, check3, [check3](bool on) {
        check3->setEnabled(on);
        check3->setChecked(on);
    });

    auto* buttons = AddOkCancel(dialog, layout);
    if (!ExecDialog(dialog, [buttons] { buttons->button(QDialogButtonBox::Ok)->click(); })) return IDCANCEL;

    // CImportModDlg::OnOK(): the text of the radio button not chosen is cleared
    if (radio2->isChecked()) dlg->m_txtradio1 = "";
    else dlg->m_txtradio2 = "";
    dlg->m_check1 = check1->isChecked();
    dlg->m_check2 = check2->isChecked();
    dlg->m_check3 = check3->isChecked();
    dlg->m_check4 = check4->isChecked();
    dlg->m_check5 = check5->isChecked();
    dlg->m_check6 = check6->isChecked();
    dlg->m_check7 = check7->isChecked();
    dlg->m_check8 = check8->isChecked();
    return IDOK;
}

static INT_PTR RunImportTmc(QWidget* parent, CImportTmcDlg* dlg)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Import Theta Music Composer module");
    auto* layout = new QVBoxLayout(&dialog);

    layout->addWidget(new QLabel(QString::fromLocal8Bit(dlg->m_info.GetString())));
    AddHeading(layout, "Instruments:");
    auto* check1 = AddCheck(layout, "Permit to use the instrument table also for vibrato and some special TMC effects.");
    AddHeading(layout, "Size optimizations:");
    auto* check6 = AddCheck(layout, "Search and build wise loops in tracks.");
    auto* check7 = AddCheck(layout, "Truncate unused parts of tracks (only if it has data saving effect).");

    auto* buttons = AddOkCancel(dialog, layout);
    if (!ExecDialog(dialog, [buttons] { buttons->button(QDialogButtonBox::Ok)->click(); })) return IDCANCEL;

    dlg->m_check1 = check1->isChecked();
    dlg->m_check6 = check6->isChecked();
    dlg->m_check7 = check7->isChecked();
    return IDOK;
}

// ---------------------------------------------------------------------------
// IDD_IMPORTMODFINISHED / IDD_IMPORTTMCFINISHED - the import result
// (CImportModFinishedDlg, CImportTmcFinishedDlg): OK only once "I understand"
// is checked, which is remembered for the next import of the same kind
// ---------------------------------------------------------------------------

static INT_PTR RunImportFinished(QWidget* parent, const char* title, const CString& info, const char* warning, bool& understood)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(title);
    auto* layout = new QVBoxLayout(&dialog);

    auto* finished = new QLabel("Import of module finished.");
    finished->setAlignment(Qt::AlignCenter);
    layout->addWidget(finished);

    QString text = QString::fromLocal8Bit(info.GetString());
    text.replace("\r\n", "\n");
    auto* infoLabel = new QLabel(text);
    infoLabel->setAlignment(Qt::AlignCenter);
    infoLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(infoLabel);

    // A top-level layout does not ask a word-wrapped label for its height at
    // the final width, so give it a width and the height that goes with it
    auto* warningLabel = new QLabel(warning);
    warningLabel->setAlignment(Qt::AlignCenter);
    warningLabel->setWordWrap(true);
    warningLabel->setFixedWidth(360);
    warningLabel->setMinimumHeight(warningLabel->heightForWidth(360));
    layout->addSpacing(6);
    layout->addWidget(warningLabel, 0, Qt::AlignHCenter);

    auto* row = new QHBoxLayout;
    auto* icon = new QLabel;
    icon->setPixmap(dialog.style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(32, 32));
    auto* check = new QCheckBox("Yes... Ok, ok... I understand.");
    check->setChecked(understood);
    row->addWidget(icon);
    row->addWidget(check);
    row->addStretch();
    layout->addLayout(row);

    auto* buttons = AddOkCancel(dialog, layout);
    QPushButton* okButton = buttons->button(QDialogButtonBox::Ok);
    okButton->setEnabled(understood);
    QObject::connect(check, &QCheckBox::toggled, okButton, [&understood, okButton](bool on) {
        understood = on;
        okButton->setEnabled(on);
    });

    auto ok = [check, okButton] { check->setChecked(true); okButton->click(); };
    return ExecDialog(dialog, ok) ? IDOK : IDCANCEL;
}

static bool g_importModUnderstood = false;          // g_importmodyesokok of importdlgs.cpp
static bool g_importTmcUnderstood = false;          // g_importtmcyesokok

// ---------------------------------------------------------------------------

INT_PTR RmtQtRunDialog(QWidget* parent, CDialog* dlg)
{
    switch (dlg->m_nIDTemplate) {
    case IDD_FILENEW: return RunFileNew(parent, static_cast<CFileNewDlg*>(dlg));
    case IDD_IMPORTMOD: return RunImportMod(parent, static_cast<CImportModDlg*>(dlg));
    case IDD_IMPORTTMC: return RunImportTmc(parent, static_cast<CImportTmcDlg*>(dlg));
    case IDD_IMPORTMODFINISHED:
        return RunImportFinished(parent, "Import ProTracker Module", static_cast<CImportModFinishedDlg*>(dlg)->m_info,
            "Please, now you have to look over all the instuments and set up proper envelopes distortions (drums, basses, etc.), also you must correct instruments tunings according to the tuning of original samples and improve them (chords, effects, noises, etc.).",
            g_importModUnderstood);
    case IDD_IMPORTTMCFINISHED:
        return RunImportFinished(parent, "Import Theta Music Composer module", static_cast<CImportTmcFinishedDlg*>(dlg)->m_info,
            "Please, now you have to look over all the instuments and whole song and check if it's all right, otherwise you must correct it manually (some special instrument effects aren't converted automatically). Also some stereo and AUDCTL events may be wrong, because of different stereo and AUDCTL conception in TMC and RMT.",
            g_importTmcUnderstood);
    default: return IDCANCEL;                       // not rewritten in Qt yet
    }
}
