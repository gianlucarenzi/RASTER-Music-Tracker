// TuningDialog.cpp : implementation file
//


#include "StdAfx.h"
#include "Rmt.h"
#include "TuningDialog.h"
#include "Tuning.h"
#include "Global.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CTuning g_Tuning;

// TuningDialog dialog
TuningDialog::TuningDialog(CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_TUNING, pParent)
{
    m_tuningSettings = {};
    m_tuningRatios = {};
}

void TuningDialog::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_BASETUNING, m_tuningSettings.basetuning);
    DDV_MinMaxDouble(pDX, m_tuningSettings.basetuning, 6.875, 7040); //should be more than enough...
    DDX_CBIndex(pDX, IDC_BASENOTE, m_tuningSettings.basenote);
    DDX_CBIndex(pDX, IDC_TEMPERAMENT, m_tuningSettings.temperament);
    // numerator values
    DDX_Text(pDX, IDC_UNISON_L, m_tuningRatios.UNISON.numerator);
    DDX_Text(pDX, IDC_MINOR_2ND_L, m_tuningRatios.MIN_2ND.numerator);
    DDX_Text(pDX, IDC_MAJOR_2ND_L, m_tuningRatios.MAJ_2ND.numerator);
    DDX_Text(pDX, IDC_MINOR_3RD_L, m_tuningRatios.MIN_3RD.numerator);
    DDX_Text(pDX, IDC_MAJOR_3RD_L, m_tuningRatios.MAJ_3RD.numerator);
    DDX_Text(pDX, IDC_PERFECT_4TH_L, m_tuningRatios.PERF_4TH.numerator);
    DDX_Text(pDX, IDC_TRITONE_L, m_tuningRatios.TRITONE.numerator);
    DDX_Text(pDX, IDC_PERFECT_5TH_L, m_tuningRatios.PERF_5TH.numerator);
    DDX_Text(pDX, IDC_MINOR_6TH_L, m_tuningRatios.MIN_6TH.numerator);
    DDX_Text(pDX, IDC_MAJOR_6TH_L, m_tuningRatios.MAJ_6TH.numerator);
    DDX_Text(pDX, IDC_MINOR_7TH_L, m_tuningRatios.MIN_7TH.numerator);
    DDX_Text(pDX, IDC_MAJOR_7TH_L, m_tuningRatios.MAJ_7TH.numerator);
    DDX_Text(pDX, IDC_OCTAVE_L, m_tuningRatios.OCTAVE.numerator);
    // denominator values
    DDX_Text(pDX, IDC_UNISON_R, m_tuningRatios.UNISON.denominator);
    DDX_Text(pDX, IDC_MINOR_2ND_R, m_tuningRatios.MIN_2ND.denominator);
    DDX_Text(pDX, IDC_MAJOR_2ND_R, m_tuningRatios.MAJ_2ND.denominator);
    DDX_Text(pDX, IDC_MINOR_3RD_R, m_tuningRatios.MIN_3RD.denominator);
    DDX_Text(pDX, IDC_MAJOR_3RD_R, m_tuningRatios.MAJ_3RD.denominator);
    DDX_Text(pDX, IDC_PERFECT_4TH_R, m_tuningRatios.PERF_4TH.denominator);
    DDX_Text(pDX, IDC_TRITONE_R, m_tuningRatios.TRITONE.denominator);
    DDX_Text(pDX, IDC_PERFECT_5TH_R, m_tuningRatios.PERF_5TH.denominator);
    DDX_Text(pDX, IDC_MINOR_6TH_R, m_tuningRatios.MIN_6TH.denominator);
    DDX_Text(pDX, IDC_MAJOR_6TH_R, m_tuningRatios.MAJ_6TH.denominator);
    DDX_Text(pDX, IDC_MINOR_7TH_R, m_tuningRatios.MIN_7TH.denominator);
    DDX_Text(pDX, IDC_MAJOR_7TH_R, m_tuningRatios.MAJ_7TH.denominator);
    DDX_Text(pDX, IDC_OCTAVE_R, m_tuningRatios.OCTAVE.denominator);
}

// clang-format off
BEGIN_MESSAGE_MAP(TuningDialog, CDialog)
    ON_BN_CLICKED(IDTESTNOW, OnClickedIdtestnow)
    ON_BN_CLICKED(IDRESET, OnClickedIdreset)
    ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()
// clang-format on

// TuningDialog message handlers
BOOL TuningDialog::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Backup all current values first
    m_tuningSettingsBackup = g_tuning;
    m_tuningRatiosBackup = g_tuningRatios;

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void TuningDialog::OnOK()
{
    OnClickedIdtestnow();

    CDialog::OnOK();
}

void TuningDialog::OnClickedIdtestnow()
{
    // Get current screen values.
    TuningDialog::UpdateData();

    g_tuning = m_tuningSettings;
    g_tuningRatios = m_tuningRatios;

    g_Tuning.InitTuning();
}

void TuningDialog::OnClickedIdreset()
{
    // Retrieve the last backed up values.
    g_tuning = m_tuningSettingsBackup;
    g_tuningRatios = m_tuningRatiosBackup;

    g_Tuning.InitTuning();
}

void TuningDialog::OnBnClickedCancel()
{
    // Reset the values from the last backup.
    OnClickedIdreset();
    // And done, nothing else to be done here.
    CDialog::OnCancel();
}
