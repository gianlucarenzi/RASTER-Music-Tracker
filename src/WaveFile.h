#pragma once

#include "PlatformTypes.h"
#ifdef RMT_HAS_MFC
#include <mmsystem.h>
#endif
// Note: outside the real-MFC build, PCMWAVEFORMAT/MMCKINFO/MMIOINFO/HMMIO
// and the mmio*() functions come from MfcTypes.h (a minimal, always-fails
// stand-in - see the comment above mmioOpen() there).

class CWaveFile
{
public:
	bool OpenFile(LPTSTR Filename, int SampleRate, int SampleSize, int Channels);
	void CloseFile();
	void WriteWave(BYTE* Data, int Size);

private:
	PCMWAVEFORMAT WaveFormat;
	MMCKINFO ckOutRIFF, ckOut;
	MMIOINFO mmioinfoOut;
	HMMIO hmmioOut;
};