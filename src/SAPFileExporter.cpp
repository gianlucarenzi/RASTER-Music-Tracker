#include "PlatformTypes.h"
#include <vector>
#include "SAPFileExporter.h"
#include "Memory.h"
#include "lzss_sap.h"
#include "LZSSFile.h"
#include "VUPlayer.h"
#include "AtariIO.h"
#include "GuiHelpers.h"
#include "Global.h"

bool CSAPFileExporter::ExportSAP_B_LZSS(CSongExport& songExport, CSAPFile& sapFile, std::ofstream& ou)
{
    byte memory[RAM_SIZE]{};
    MemoryAddress addressFrom;
    MemoryAddress addressTo;

    auto binaryFilePath = GetResourceFilePath(std::filesystem::path("resources/players"), "vu_player_v2.obx");
    if (!CAtariIO::LoadBinaryFile(binaryFilePath, memory, addressFrom, addressTo)) {
        CString message;
        message.Format("Fatal error with RMT LZSS system routines.\nCouldn't load '%s'.", binaryFilePath);
        SendErrorMessage("Export aborted", message);
        return false;
    }

    SetStatusBarText("Compressing data ...");

    const int frameSize = CLZSSFile::GetFrameSize(songExport.GetSong());

    // LZSS buffers for the intro and loop parts of the tune being reconstructed.
    // Large and on the heap: LZSS_SAP() writes as much as the compression
    // yields, and a stereo song's stream compressed into a 64K stack buffer
    // overran it. The former "full" compression of the whole stream is dropped
    // with it - its result was never used.
    const size_t LZSS_BUFFER_SIZE = 0xFFFFF;
    std::vector<byte> buff2(LZSS_BUFFER_SIZE);
    std::vector<byte> buff3(LZSS_BUFFER_SIZE);

    CCompressLzss lzssData;

    // Now, create LZSS files using the SAP-R dump created earlier
    const CPokeyStream& pokeyStream = songExport.GetPokeyStream();
    int intro = lzssData.LZSS_SAP(pokeyStream.GetConstStreamBuffer(), pokeyStream.GetThirdCountPoint() * frameSize, buff2.data());
    int loop = lzssData.LZSS_SAP(pokeyStream.GetConstStreamBuffer() + (pokeyStream.GetFirstCountPoint() * frameSize), pokeyStream.GetSecondCountPoint() * frameSize, buff3.data());

    // Some additional variables that will be used below
    int targetAddrOfModule = VUPlayer::SONGDATA; // All the LZSS data will be written starting from this address
    //int lzss_offset = (intro) ? targetAddrOfModule + intro : targetAddrOfModule + full;	// Calculate the offset for the export process between the subtune parts, at the moment only 1 tune at the time can be exported
    int lzss_offset = (intro > 16) ? targetAddrOfModule + intro : targetAddrOfModule;
    int lzss_end = lzss_offset + loop; // this sets the address that defines where the data stream has reached its end

    ClearStatusBar();

    // If the size is too big, abort the process and show an error message
    // JAC! Have same error handling
    if (lzss_end > RAM_MAX_ADDRESS) {
        SendErrorMessage("Buffer Overflow",
                         "Error, LZSS data is too big to fit in memory!\n\n"
                         "High Instrument Speed and/or Stereo greatly inflate memory usage, even when data is compressed");
        return false;
    }

    sapFile.SetInitAddress(VUPlayer::INIT_SAP);
    sapFile.SetPlayerAddress(VUPlayer::DO_PLAY_ADDR);

    sapFile.Export(ou);

    VUPlayer::PatchMemoryForSAP_B(memory, songExport.GetSong(), buff2.data(), buff3.data(), intro, loop, targetAddrOfModule, lzss_offset, lzss_end);


    // Reconstruct the export binary
    CAtariIO::SaveBinaryBlock(ou, memory, 0x1900, 0x1EFF, true);  // LZSS Driver, and some free bytes for later if needed
    CAtariIO::SaveBinaryBlock(ou, memory, 0x2000, 0x27FF, false); // VUPlayer only

    // Overwrite the LZSS data region with both the pointers for subtunes index, and the actual LZSS streams until the end of file
    CAtariIO::SaveBinaryBlock(ou, memory, VUPlayer::LZSS_POINTER, lzss_end, false);

    return true;
}

bool CSAPFileExporter::ExportSAP_R(CSongExport& songExport, CSAPFile& sapFile, std::ofstream& ou)
{
    sapFile.SetType("R");
    sapFile.Export(ou);

    // Write the SAP-R stream to the output file defined in the path dialog with the data specified above
    const CPokeyStream& pokeyStream = songExport.GetPokeyStream();
    pokeyStream.WriteToFile(ou, pokeyStream.GetFirstCountPoint(), 0);

    return true;
}