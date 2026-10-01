#pragma once

#include "Song.h"
#include "ASMFile.h"

// The answers of the relocatable ASM export dialog, so that the export itself
// (ExportAsRelocatableAsmForRmtPlayerApply()) runs independently of the dialog.
struct TRelocatableAsmExportParams {
    CString strAsmLabelForStartOfSong;
    BOOL wantRelocatableInstruments;
    BOOL wantRelocatableTracks;
    BOOL wantRelocatableSongLines;
    CString strAsmInstrumentsLabel;
    CString strAsmTracksLabel;
    CString strAsmSongLinesLabel;
    AssemblerFormat assemblerFormat;
    BOOL sfxSupport;
    BOOL globalVolumeFade;
    BOOL noStartingSongLine;
};

class CASMFileExporter {
public:
    /// <summary>
    /// Export the RMT module as assembler
    /// </summary>
    /// <param name="ou">Output stream</param>
    /// <param name="exportStrippedDesc">Data about the packed RMT module</param>
    /// <returns>true if the save went OK</returns>
    static bool ExportAsAsm(const CSong& song, std::ofstream& ou, TExportDescription* exportStrippedDesc);
    // The dialog-independent half: exportType 1 = tracks, 2 = whole song; notesIndexOrFreq 1 = notes; durationsType 1..3
    static bool ExportAsAsmApply(const CSong& song, std::ofstream& ou, int exportType, int notesIndexOrFreq, int durationsType);

    /// <summary>
    /// Export the RMT data for the RMTPlayer assembler player.
    /// All data is exported: instruments, tracks, song lines
    /// </summary>
    /// <param name="ou">Output stream</param>
    /// <param name="exportDescStripped">Data about the packed RMT module</param>
    /// <returns>true if the save went OK</returns>
    static bool ExportAsRelocatableAsmForRmtPlayer(CSong& song, std::ofstream& ou, TExportDescription* exportStrippedDesc);
    // The dialog-independent half
    static bool ExportAsRelocatableAsmForRmtPlayerApply(CSong& song, std::ofstream& ou, TExportDescription* exportDescStripped, TExportDescription* exportDescWithSFX, const TRelocatableAsmExportParams& params);


    // TODO: Used by export dialog
    void static ComposeRMTFEATstring(const CSong& song, CString& dest, const char* filename, BYTE* instrumentSavedFlags, BYTE* trackSavedFlags, BOOL sfx, BOOL gvf, BOOL nos, AssemblerFormat assemblerFormat);

    static BOOL BuildRelocatableAsm(
        const CSong& song, CString& dest,
        TExportDescription* exportDesc,
        CString strAsmStartLabel,
        CString strTracksLabel,
        CString strSongLinesLabel,
        CString strInstrumentsLabel,
        AssemblerFormat assemblerFormat,
        BOOL sfx,
        BOOL gvf,
        BOOL nos,
        bool bWantSizeInfoOnly);

private:
};
