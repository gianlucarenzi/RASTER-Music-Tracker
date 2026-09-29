
#include "PlatformTypes.h"
#include "WaveFile.h"

#ifdef RMT_HAS_MFC

bool CWaveFile::OpenFile(LPTSTR Filename, int SampleRate, int SampleSize, int Channels)
{
	int nError;

	WaveFormat.wf.wFormatTag = WAVE_FORMAT_PCM;
	WaveFormat.wf.nChannels = Channels;	
	WaveFormat.wf.nSamplesPerSec = SampleRate;
	WaveFormat.wBitsPerSample = SampleSize;
	WaveFormat.wf.nBlockAlign = (WaveFormat.wBitsPerSample / 8) * WaveFormat.wf.nChannels;
	WaveFormat.wf.nAvgBytesPerSec = WaveFormat.wf.nSamplesPerSec * WaveFormat.wf.nBlockAlign;

	hmmioOut = mmioOpen(Filename, NULL, MMIO_ALLOCBUF | MMIO_READWRITE | MMIO_CREATE);

	ckOutRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	ckOutRIFF.cksize = 0;

	nError = mmioCreateChunk(hmmioOut, &ckOutRIFF, MMIO_CREATERIFF);

	if (nError != MMSYSERR_NOERROR)
		return false;

	ckOut.ckid = mmioFOURCC('f', 'm', 't', ' ');
	ckOut.cksize = sizeof(PCMWAVEFORMAT);

	nError = mmioCreateChunk(hmmioOut, &ckOut, 0);

	if (nError != MMSYSERR_NOERROR)
		return false;

	mmioWrite(hmmioOut, (HPSTR)&WaveFormat, sizeof(PCMWAVEFORMAT));
	mmioAscend(hmmioOut, &ckOut, 0);

	ckOut.ckid = mmioFOURCC('d', 'a', 't', 'a');
	ckOut.cksize = 0;

	nError = mmioCreateChunk(hmmioOut, &ckOut, 0);

	if (nError != MMSYSERR_NOERROR)
		return false;

	mmioGetInfo(hmmioOut, &mmioinfoOut, 0);

	return true;
}

void CWaveFile::CloseFile()
{
	mmioinfoOut.dwFlags |= MMIO_DIRTY;
	mmioSetInfo(hmmioOut, &mmioinfoOut, 0);
	mmioAscend(hmmioOut, &ckOut, 0);
	mmioAscend(hmmioOut, &ckOutRIFF, 0);
	mmioSeek(hmmioOut, 0, SEEK_SET);
	mmioDescend(hmmioOut, &ckOutRIFF, NULL, 0);
	mmioClose(hmmioOut, 0);
}

void CWaveFile::WriteWave(BYTE* Data, int Size)
{
	for (int i = 0; i < Size; i++)
	{
		if (mmioinfoOut.pchNext == mmioinfoOut.pchEndWrite)
		{
			mmioinfoOut.dwFlags |= MMIO_DIRTY;
			mmioAdvance(hmmioOut, &mmioinfoOut, MMIO_WRITE);
		}
		*((BYTE*)mmioinfoOut.pchNext) = *((BYTE*)Data + i);
		mmioinfoOut.pchNext++;
	}
}

#else

// Little-endian values of the RIFF header
static void Put16(std::ofstream& out, uint32_t v) { out.put((char)(v & 0xff)); out.put((char)((v >> 8) & 0xff)); }
static void Put32(std::ofstream& out, uint32_t v) { Put16(out, v & 0xffff); Put16(out, v >> 16); }

bool CWaveFile::OpenFile(LPTSTR Filename, int SampleRate, int SampleSize, int Channels)
{
	m_out.open(Filename, std::ios::binary | std::ios::trunc);
	if (!m_out) return false;

	uint32_t blockAlign = (SampleSize / 8) * Channels;
	m_dataSize = 0;
	m_out.write("RIFF", 4);
	Put32(m_out, 0);						// RIFF size, set by CloseFile()
	m_out.write("WAVEfmt ", 8);
	Put32(m_out, 16);						// PCMWAVEFORMAT
	Put16(m_out, WAVE_FORMAT_PCM);
	Put16(m_out, Channels);
	Put32(m_out, SampleRate);
	Put32(m_out, SampleRate * blockAlign);	// nAvgBytesPerSec
	Put16(m_out, blockAlign);
	Put16(m_out, SampleSize);
	m_out.write("data", 4);
	Put32(m_out, 0);						// data size, set by CloseFile()
	return (bool)m_out;
}

void CWaveFile::CloseFile()
{
	if (!m_out.is_open()) return;
	if (m_dataSize & 1) m_out.put(0);		// chunks are padded to an even size
	m_out.seekp(4);
	Put32(m_out, 36 + m_dataSize + (m_dataSize & 1));
	m_out.seekp(40);
	Put32(m_out, m_dataSize);
	m_out.close();
}

void CWaveFile::WriteWave(BYTE* Data, int Size)
{
	m_out.write((const char*)Data, Size);
	m_dataSize += Size;
}

#endif
