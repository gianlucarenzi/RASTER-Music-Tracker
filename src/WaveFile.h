#pragma once

#include "PlatformTypes.h"
#ifdef RMT_HAS_MFC
#include <mmsystem.h>
#endif
// Outside the real-MFC build there is no mmio*() API: the WAV file is
// written with std::ofstream (RIFF header, "fmt " and "data" chunks).
#ifndef RMT_HAS_MFC
#include <cstdint>
#include <fstream>
#endif

class CWaveFile
{
public:
	bool OpenFile(LPTSTR Filename, int SampleRate, int SampleSize, int Channels);
	void CloseFile();
	void WriteWave(BYTE* Data, int Size);

private:
#ifdef RMT_HAS_MFC
	PCMWAVEFORMAT WaveFormat;
	MMCKINFO ckOutRIFF, ckOut;
	MMIOINFO mmioinfoOut;
	HMMIO hmmioOut;
#else
	std::ofstream m_out;
	uint32_t m_dataSize = 0;
#endif
};