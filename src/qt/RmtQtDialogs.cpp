// RmtQtDialogs.cpp - the MFC dialogs of RMT, rewritten with Qt (see RmtQtDialogs.h)

#include "RmtQtDialogs.h"

#include "resource.h"
#include "Global.h"
#include "TrackTypes.h"
#include "filenewdlg.h"
#include "importdlgs.h"
#include "Song.h"
#include "exportdlgs.h"
#include "SAPFileExportDialog.h"
#include "ASMFileExporter.h"
#include "Notes.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileInfo>
#include <QFontDatabase>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpressionValidator>
#include <QScrollBar>
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
// Export options (exportdlgs.cpp, SAPFileExportDialog.cpp)
// ---------------------------------------------------------------------------

static QString FromCString(const CString& s) { return QString::fromLocal8Bit(s.GetString()); }
static CString ToCString(const QString& s) { return CString(s.toLocal8Bit().constData()); }

// A line edit that turns what is typed to upper case (ES_UPPERCASE)
static QLineEdit* UpperCaseEdit(const QString& text)
{
    auto* edit = new QLineEdit(text.toUpper());
    QObject::connect(edit, &QLineEdit::textEdited, edit, [edit](const QString& t) {
        if (t != t.toUpper()) {
            int pos = edit->cursorPosition();
            edit->setText(t.toUpper());
            edit->setCursorPosition(pos);
        }
    });
    return edit;
}

// A fixed pitch font; the system one may not be (e.g. QT_QPA_PLATFORM=offscreen)
static QFont MonoFont()
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    if (!QFontInfo(font).fixedPitch()) {
        font.setFamily("Monospace");
        font.setStyleHint(QFont::TypeWriter);
    }
    return font;
}

// A read-only text box for generated assembler text
static QPlainTextEdit* AsmTextBox()
{
    auto* text = new QPlainTextEdit;
    text->setReadOnly(true);
    text->setLineWrapMode(QPlainTextEdit::NoWrap);
    text->setFont(MonoFont());
    text->setTabStopDistance(QFontMetricsF(text->font()).horizontalAdvance(' ') * 8);
    text->setMinimumSize(560, 200);
    return text;
}

static QComboBox* AsmFormatCombo(AssemblerFormat& format)
{
    auto* combo = new QComboBox;
    combo->addItems({ "Atasm", "Xasm" });           // index = AssemblerFormat
    if (format < ATASM) format = ATASM;
    if (format > XASM) format = XASM;
    combo->setCurrentIndex(format);
    return combo;
}

static void ClickOk(QDialogButtonBox* buttons) { buttons->button(QDialogButtonBox::Ok)->click(); }

// IDD_EXPORT_STRIPPED_RMT - CExportStrippedRMTDialog
static INT_PTR RunExportStrippedRmt(QWidget* parent, CExportStrippedRMTDialog* dlg)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Export RMT stripped file");
    auto* layout = new QVBoxLayout(&dialog);

    layout->addWidget(new QLabel("Memory location"));
    auto* address = new QLineEdit(QString::asprintf("%04X", dlg->m_exportAddr));
    address->setMaxLength(4);
    address->setMaximumWidth(60);
    address->setValidator(new QRegularExpressionValidator(QRegularExpression("[0-9A-Fa-f]{0,4}"), address));
    QObject::connect(address, &QLineEdit::textEdited, address, [address](const QString& t) {
        int pos = address->cursorPosition();
        address->setText(t.toUpper());
        address->setCursorPosition(pos);
    });
    auto* info = new QLabel;
    auto* addressRow = new QHBoxLayout;
    addressRow->addWidget(new QLabel("From address (HEX):"));
    addressRow->addWidget(address);
    addressRow->addWidget(info, 1, Qt::AlignCenter);
    layout->addLayout(addressRow);

    auto* format = AsmFormatCombo(dlg->m_assemblerFormat);
    auto* formatRow = new QHBoxLayout;
    formatRow->addWidget(new QLabel("Assembler format:"));
    formatRow->addWidget(format);
    formatRow->addStretch();
    layout->addLayout(formatRow);

    auto* sfx = new QCheckBox("SFX support (also preserve unused tracks and instruments in module)");
    auto* gvf = new QCheckBox("GlobalVolumeFade support (RMTGLOBALVOLUMEFADE variable)");
    auto* nos = new QCheckBox("No songline start (always start from songline 0)");
    sfx->setChecked(dlg->m_sfxSupport);
    gvf->setChecked(dlg->m_globalVolumeFade);
    nos->setChecked(dlg->m_noStartingSongLine);
    layout->addWidget(sfx);
    layout->addWidget(gvf);
    layout->addWidget(nos);

    layout->addWidget(new QLabel("RMT FEATures definitions (for optimizations of RMT player assembler routine)"));
    auto* rmtfeat = AsmTextBox();
    layout->addWidget(rmtfeat);
    auto* copy = new QPushButton("Copy all to clipboard");
    QObject::connect(copy, &QPushButton::clicked, rmtfeat, [rmtfeat] {
        QGuiApplication::clipboard()->setText(rmtfeat->toPlainText());
    });
    layout->addWidget(copy, 0, Qt::AlignRight);

    auto* warning = new QLabel;
    warning->setWordWrap(true);
    layout->addWidget(warning);

    // CExportStrippedRMTDialog::ChangeParams(): the address is kept inside
    // the 64 KB, the RMTFEAT definitions follow the options
    auto changeParams = [&] {
        int adr = (int)strtoul(address->text().toLatin1().constData(), nullptr, 16);
        dlg->m_assemblerFormat = (AssemblerFormat)format->currentIndex();
        dlg->m_sfxSupport = sfx->isChecked();
        int length = dlg->m_sfxSupport ? dlg->m_moduleLengthForSFX : dlg->m_moduleLengthForStrippedRMT;
        if (adr > 0x10000 - length) adr = 0x10000 - length;
        dlg->m_exportAddr = adr;
        info->setText(QString::asprintf("=>  $%04X - $%04X , length $%04X (%u bytes)", adr, adr + length - 1, length, length));
        warning->setText(dlg->m_sfxSupport
            ? "Warning:\nThis output file doesn't contain song name and names of all instruments."
            : "Warning:\nThis output file doesn't contain any unused or empty tracks and instruments, song name and names of all instruments.");
        dlg->m_globalVolumeFade = gvf->isChecked();
        dlg->m_noStartingSongLine = nos->isChecked();

        BYTE* instrsav = dlg->m_sfxSupport ? dlg->m_savedInstrFlagsForSFX : dlg->m_savedInstrFlagsForStrippedRMT;
        BYTE* tracksav = dlg->m_sfxSupport ? dlg->m_savedTracksFlagsForSFX : dlg->m_savedTracksFlagsForStrippedRMT;
        CString s;
        CASMFileExporter::ComposeRMTFEATstring(*dlg->m_song, s, dlg->m_filename, instrsav, tracksav,
            dlg->m_sfxSupport, dlg->m_globalVolumeFade, dlg->m_noStartingSongLine, dlg->m_assemblerFormat);
        rmtfeat->setPlainText(FromCString(s).replace("\r\n", "\n"));
    };
    QObject::connect(address, &QLineEdit::textChanged, &dialog, changeParams);
    QObject::connect(format, QOverload<int>::of(&QComboBox::currentIndexChanged), &dialog, changeParams);
    QObject::connect(sfx, &QCheckBox::toggled, &dialog, changeParams);
    QObject::connect(gvf, &QCheckBox::toggled, &dialog, changeParams);
    QObject::connect(nos, &QCheckBox::toggled, &dialog, changeParams);
    changeParams();

    auto* buttons = AddOkCancel(dialog, layout);
    return ExecDialog(dialog, [buttons] { ClickOk(buttons); }) ? IDOK : IDCANCEL;
}

// IDD_EXPMSX - CExpMSXDlg (XEX with the LZSS driver); the options are kept
// for the next export like g_msxcheck, g_msx_shuffle, g_region_auto, g_msxcol
static bool s_msxRasterbar = true;
static bool s_msxShuffle = true;
static bool s_msxRegionAuto = true;
static int s_msxColor = 6;

static INT_PTR RunExportXex(QWidget* parent, CExpMSXDlg* dlg)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Export Atari executable MSX");
    auto* layout = new QVBoxLayout(&dialog);

    layout->addWidget(new QLabel("Text displayed on screen during music playback. 4+1 lines, 40 characters per line.\n"
        "The 5th line of text is shown instead of 4th line when the Shift key is held down."));
    auto* edit = new QPlainTextEdit(FromCString(dlg->m_txt).remove('\r'));
    edit->setLineWrapMode(QPlainTextEdit::NoWrap);
    edit->setFont(MonoFont());
    edit->setMinimumHeight(110);
    layout->addWidget(edit);

    auto* previewLabel = new QLabel("MSX screen preview");
    previewLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(previewLabel);
    auto* preview = new QPlainTextEdit;
    preview->setReadOnly(true);
    preview->setLineWrapMode(QPlainTextEdit::NoWrap);
    preview->setFont(MonoFont());
    preview->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    preview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QFontMetrics fm(preview->font());
    preview->setFixedSize(fm.horizontalAdvance(QString(40, 'W')) + 16, fm.lineSpacing() * 4 + 16);
    layout->addWidget(preview, 0, Qt::AlignHCenter);
    auto* shiftTest = new QPushButton("Atari SHIFTkey test");
    shiftTest->setCheckable(true);
    layout->addWidget(shiftTest, 0, Qt::AlignHCenter);

    auto* rasterbar = new QCheckBox("Display rasterbar for CPU usage");
    auto* shuffle = new QCheckBox("Shuffle the rasterbar colors");
    rasterbar->setChecked(s_msxRasterbar);
    shuffle->setChecked(s_msxShuffle);
    auto* checkRow = new QHBoxLayout;
    checkRow->addWidget(rasterbar);
    checkRow->addWidget(shuffle);
    layout->addLayout(checkRow);

    auto* color = new QScrollBar(Qt::Horizontal);
    color->setRange(1, 127);
    color->setPageStep(8);
    color->setValue(s_msxColor / 2);
    auto* colorInfo = new QLabel;
    colorInfo->setMinimumWidth(160);
    auto* colorRow = new QHBoxLayout;
    colorRow->addWidget(new QLabel("Color:"));
    colorRow->addWidget(color, 1);
    colorRow->addWidget(colorInfo);
    layout->addLayout(colorRow);

    layout->addWidget(new QLabel(FromCString(dlg->m_speedinfo)));
    auto* regionAuto = new QCheckBox("Automatically adjust playback speed");
    regionAuto->setChecked(s_msxRegionAuto);
    layout->addWidget(regionAuto);

    // CExpMSXDlg::OnHScroll(): the color is 2 x the scroll position
    auto changeColor = [&](int c) {
        static const char* bar[] = { "Gray","Rust","Orange","Red-orange","Pink","Purple","Cobalt blue","Blue",
            "Medium blue","Dark blue","Blue-grey","Olive green","Medium green","Dark green","Orange-green","Brown" };
        s_msxColor = c * 2;
        colorInfo->setText(QString::asprintf("%i = %s %i", s_msxColor, bar[s_msxColor / 16], s_msxColor % 16));
    };
    QObject::connect(color, &QScrollBar::valueChanged, &dialog, changeColor);
    changeColor(color->value());

    // CExpMSXDlg::ChangeParams(): 5 lines of up to 40 characters; the preview
    // shows lines 1-4, or 1-3 and 5 while the SHIFT key test is on
    auto changeParams = [&] {
        QString s = edit->toPlainText(), d, d4th, d5th;
        int from = 0, line = 0;
        while (from < s.length() && line < 5) {
            int i = s.indexOf('\n', from);
            if (i >= 0) {
                QString l = s.mid(from, std::min(i - from, 40)) + "\r\n";
                d += l;
                if (line != 4) d4th += l;
                if (line != 3) d5th += l;
                from = i + 1;
                line++;
            }
            else {
                QString l = s.mid(from, 40);
                d += l;
                if (line != 4) d4th += l;
                if (line != 3) d5th += l;
                break;
            }
        }
        preview->setPlainText((shiftTest->isChecked() ? d5th : d4th).remove('\r'));
        dlg->m_txt = ToCString(d);
        bool meter = rasterbar->isChecked();
        color->setEnabled(meter);
        colorInfo->setEnabled(meter);
        shuffle->setEnabled(meter);
    };
    QObject::connect(edit, &QPlainTextEdit::textChanged, &dialog, changeParams);
    QObject::connect(shiftTest, &QPushButton::toggled, &dialog, changeParams);
    QObject::connect(rasterbar, &QCheckBox::toggled, &dialog, changeParams);
    changeParams();

    auto* buttons = AddOkCancel(dialog, layout);
    if (!ExecDialog(dialog, [buttons] { ClickOk(buttons); })) return IDCANCEL;

    // CExpMSXDlg::OnOK() and its DDX
    dlg->m_metercolor = s_msxColor;
    s_msxShuffle = shuffle->isChecked();
    s_msxRegionAuto = regionAuto->isChecked();
    s_msxRasterbar = rasterbar->isChecked();
    dlg->m_meter = rasterbar->isChecked();
    dlg->m_msx_shuffle = shuffle->isChecked();
    dlg->m_region_auto = regionAuto->isChecked();
    return IDOK;
}

// IDD_EXPORT_ASM - CExportAsmDlg (ASM simple notation)
static INT_PTR RunExportAsm(QWidget* parent, CExportAsmDlg* dlg)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Export ASM simple notation source file");
    auto* layout = new QVBoxLayout(&dialog);

    // Three groups of radio buttons; the first of each is the default
    auto addGroup = [&](const char* title, std::initializer_list<QString> texts) {
        layout->addWidget(new QLabel(title));
        auto* group = new QButtonGroup(&dialog);
        int id = 1;
        for (const QString& text : texts) {
            auto* radio = new QRadioButton(text);
            group->addButton(radio, id++);
            layout->addWidget(radio);
        }
        group->button(1)->setChecked(true);
        return group;
    };
    auto* exportType = addGroup("Export type", { "Tracks", "Whole song by song columns" });
    auto* noteValues = addGroup("Note values", { QString::asprintf("Note indexes $00-$%02X", CNotes::NOTESNUM - 1),
        "Note frequencies according to distortion in first envelope column" });
    auto* durations = addGroup("Note durations", { "Notes only (special value XXX in empty beats)",
        "Pairs of note,duration", "Pairs of duration,note" });

    layout->addWidget(new QLabel("Generate labels with prefix (empty prefix => no labels)"));
    auto* prefix = new QLineEdit(FromCString(dlg->m_prefixForAllAsmLabels));
    prefix->setMaxLength(32);                       // DDV_MaxChars(32)
    prefix->setMaximumWidth(200);
    layout->addWidget(prefix);

    auto* buttons = AddOkCancel(dialog, layout);
    if (!ExecDialog(dialog, [buttons] { ClickOk(buttons); })) return IDCANCEL;

    // CExportAsmDlg::OnOK(): 1-based index of the chosen button of each group
    dlg->m_exportType = exportType->checkedId();
    dlg->m_notesIndexOrFreq = noteValues->checkedId();
    dlg->m_durationsType = durations->checkedId();
    dlg->m_prefixForAllAsmLabels = ToCString(prefix->text());
    return IDOK;
}

// IDD_EXPORT_RMTPLAYER_ASM - CExportRelocatableAsmForRmtPlayer
static INT_PTR RunExportRelocatableAsm(QWidget* parent, CExportRelocatableAsmForRmtPlayer* dlg)
{
    QDialog dialog(parent);
    dialog.setWindowTitle("Export ASM for RmtPlayer.asm");
    auto* layout = new QVBoxLayout(&dialog);

    layout->addWidget(new QLabel("The RMT song will be exported as byte definitions with specific labels.\n"
        "The code is fully relocatable and can be split over various locations in memory!"));

    // CExportRelocatableAsmForRmtPlayer::OnInitDialog(): default labels
    if (dlg->m_strAsmLabelForStartOfSong.IsEmpty()) dlg->m_strAsmLabelForStartOfSong = "RMT_SONG_DATA";
    if (dlg->m_strAsmTracksLabel.IsEmpty()) dlg->m_strAsmTracksLabel = "RMT_SONG_TRACKS";
    if (dlg->m_strAsmSongLinesLabel.IsEmpty()) dlg->m_strAsmSongLinesLabel = "RMT_SONG_LINES";
    if (dlg->m_strAsmInstrumentsLabel.IsEmpty()) dlg->m_strAsmInstrumentsLabel = "RMT_INSTRUMENT_DATA";

    auto* grid = new QGridLayout;
    auto* songLabel = UpperCaseEdit(FromCString(dlg->m_strAsmLabelForStartOfSong));
    grid->addWidget(new QLabel("ASM Label for start of song data:"), 0, 0, 1, 2);
    grid->addWidget(songLabel, 0, 2);
    auto addRelocation = [&](int row, const char* check, const char* label, const CString& value, BOOL on) {
        auto* box = new QCheckBox(check);
        box->setChecked(on);
        auto* edit = UpperCaseEdit(FromCString(value));
        auto* text = new QLabel(label);
        text->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        grid->addWidget(box, row, 0);
        grid->addWidget(text, row, 1);
        grid->addWidget(edit, row, 2);
        return std::make_pair(box, edit);
    };
    auto [instrCheck, instrEdit] = addRelocation(1, "Relocate instruments to", "ASM Instruments Label:", dlg->m_strAsmInstrumentsLabel, dlg->m_wantRelocatableInstruments);
    auto [tracksCheck, tracksEdit] = addRelocation(2, "Relocate tracks to", "ASM Tracks Label:", dlg->m_strAsmTracksLabel, dlg->m_wantRelocatableTracks);
    auto [songCheck, songEdit] = addRelocation(3, "Relocatable song lines to", "ASM Song Label:", dlg->m_strAsmSongLinesLabel, dlg->m_wantRelocatableSongLines);
    layout->addLayout(grid);

    auto* format = AsmFormatCombo(dlg->m_assemblerFormat);
    auto* formatRow = new QHBoxLayout;
    formatRow->addWidget(new QLabel("Assembler format:"));
    formatRow->addWidget(format);
    formatRow->addStretch();
    layout->addLayout(formatRow);

    auto* sfx = new QCheckBox("SFX support (also preserve unused tracks and instruments in module)");
    auto* gvf = new QCheckBox("GlobalVolumeFade support (RMTGLOBALVOLUMEFADE variable)");
    auto* nos = new QCheckBox("No songline start (always start from songline 0)");
    sfx->setChecked(dlg->m_sfxSupport);
    gvf->setChecked(dlg->m_globalVolumeFade);
    nos->setChecked(dlg->m_noStartingSongLine);
    layout->addWidget(sfx);
    layout->addWidget(gvf);
    layout->addWidget(nos);

    auto* info = new QLabel;
    info->setAlignment(Qt::AlignCenter);
    layout->addWidget(info);
    auto* sizes = AsmTextBox();
    layout->addWidget(sizes);

    // CExportRelocatableAsmForRmtPlayer::ChangeParams(): label edits follow
    // their check box, the text shows the size of each part
    auto changeParams = [&] {
        dlg->m_wantRelocatableTracks = tracksCheck->isChecked();
        dlg->m_wantRelocatableSongLines = songCheck->isChecked();
        dlg->m_wantRelocatableInstruments = instrCheck->isChecked();
        tracksEdit->setEnabled(dlg->m_wantRelocatableTracks);
        songEdit->setEnabled(dlg->m_wantRelocatableSongLines);
        instrEdit->setEnabled(dlg->m_wantRelocatableInstruments);
        dlg->m_strAsmLabelForStartOfSong = ToCString(songLabel->text());
        dlg->m_strAsmTracksLabel = ToCString(tracksEdit->text());
        dlg->m_strAsmSongLinesLabel = ToCString(songEdit->text());
        dlg->m_strAsmInstrumentsLabel = ToCString(instrEdit->text());

        dlg->m_sfxSupport = sfx->isChecked();
        TExportDescription* desc = dlg->m_sfxSupport ? dlg->m_exportDescWithSFX : dlg->m_exportDescStripped;
        int len = desc->firstByteAfterModule - desc->targetAddrOfModule;
        info->setText(QString::asprintf("Length $%04X (%u bytes)", len, len));

        dlg->m_assemblerFormat = (AssemblerFormat)format->currentIndex();
        dlg->m_globalVolumeFade = gvf->isChecked();
        dlg->m_noStartingSongLine = nos->isChecked();

        CString s;
        CASMFileExporter::BuildRelocatableAsm(*dlg->m_song, s, desc, "",
            dlg->m_wantRelocatableTracks ? dlg->m_strAsmTracksLabel : CString(""),
            dlg->m_wantRelocatableSongLines ? dlg->m_strAsmSongLinesLabel : CString(""),
            dlg->m_wantRelocatableInstruments ? dlg->m_strAsmInstrumentsLabel : CString(""),
            dlg->m_assemblerFormat, dlg->m_sfxSupport, false, false,
            true);                                  // just the size info
        sizes->setPlainText(FromCString(s).replace("\r\n", "\n"));
    };
    for (QLineEdit* edit : { songLabel, instrEdit, tracksEdit, songEdit })
        QObject::connect(edit, &QLineEdit::textChanged, &dialog, changeParams);
    for (QCheckBox* box : { instrCheck, tracksCheck, songCheck, sfx, gvf, nos })
        QObject::connect(box, &QCheckBox::toggled, &dialog, changeParams);
    QObject::connect(format, QOverload<int>::of(&QComboBox::currentIndexChanged), &dialog, changeParams);
    changeParams();

    auto* buttons = AddOkCancel(dialog, layout);
    return ExecDialog(dialog, [buttons] { ClickOk(buttons); }) ? IDOK : IDCANCEL;
}

// IDD_EXPSAP - CSAPFileExportDialog (SAP-R, SAP with the LZSS driver)
static INT_PTR RunExportSap(QWidget* parent, CSAPFileExportDialog* dlg)
{
    QDialog dialog(parent);
    dialog.setWindowTitle(FromCString(dlg->m_title));
    dialog.setMinimumWidth(560);
    auto* layout = new QVBoxLayout(&dialog);

    auto addField = [&](const char* label, const CString& value) {
        auto* text = new QLabel(label);
        text->setWordWrap(true);
        layout->addWidget(text);
        auto* edit = new QLineEdit(FromCString(value));
        layout->addWidget(edit);
        return edit;
    };
    auto* name = addField("NAME", dlg->m_name);
    auto* author = addField("AUTHOR", dlg->m_author);
    auto* date = addField("DATE", dlg->m_date);
    date->setMaximumWidth(120);
    auto* subsongs = addField("Subsongs (hexadecimal song line number for each subsong, first is default song):", dlg->m_subsongs);

    auto* buttons = AddOkCancel(dialog, layout);
    if (!ExecDialog(dialog, [buttons] { ClickOk(buttons); })) return IDCANCEL;

    dlg->m_name = ToCString(name->text());
    dlg->m_author = ToCString(author->text());
    dlg->m_date = ToCString(date->text());
    dlg->m_subsongs = ToCString(subsongs->text());
    return IDOK;
}

// ---------------------------------------------------------------------------

INT_PTR RmtQtRunDialog(QWidget* parent, CDialog* dlg)
{
    switch (dlg->m_nIDTemplate) {
    case IDD_FILENEW: return RunFileNew(parent, static_cast<CFileNewDlg*>(dlg));
    case IDD_IMPORTMOD: return RunImportMod(parent, static_cast<CImportModDlg*>(dlg));
    case IDD_EXPORT_STRIPPED_RMT: return RunExportStrippedRmt(parent, static_cast<CExportStrippedRMTDialog*>(dlg));
    case IDD_EXPMSX: return RunExportXex(parent, static_cast<CExpMSXDlg*>(dlg));
    case IDD_EXPORT_ASM: return RunExportAsm(parent, static_cast<CExportAsmDlg*>(dlg));
    case IDD_EXPORT_RMTPLAYER_ASM: return RunExportRelocatableAsm(parent, static_cast<CExportRelocatableAsmForRmtPlayer*>(dlg));
    case IDD_EXPSAP: return RunExportSap(parent, static_cast<CSAPFileExportDialog*>(dlg));
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
