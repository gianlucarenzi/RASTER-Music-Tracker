#pragma once

#include "PlatformTypes.h"
// Outside the real-MFC build there is no mmio*() API: the WAV file is
// written with std::ofstream (RIFF header, "fmt " and "data" chunks).
#include <cstdint>
#include <fstream>

class CWaveFile {
public:
    bool OpenFile(LPTSTR Filename, int SampleRate, int SampleSize, int Channels);
    void CloseFile();
    void WriteWave(BYTE* Data, int Size);

private:
    std::ofstream m_out;
    uint32_t m_dataSize = 0;
};