//
// DirectSoundAudio.cpp - DirectSound backend implementation
//

#include "DirectSoundAudio.h"
#include <cstring>

#ifdef _WIN32
    #include <windows.h>
    #include <dsound.h>
    #pragma comment(lib, "dsound.lib")
    #pragma comment(lib, "dxguid.lib")
#endif

DirectSoundAudio::DirectSoundAudio()
    : m_lpds(nullptr), m_lpdsbPrimary(nullptr), m_is_playing(false),
      m_is_initialized(false), m_write_pos(0), m_play_pos(0)
{
    memset(m_buffer, 0, sizeof(m_buffer));
}

DirectSoundAudio::~DirectSoundAudio()
{
    Deinit();
}

bool DirectSoundAudio::Init(const AudioFormat& format)
{
    if (!format.IsValid())
    {
        m_error_message = "Invalid audio format";
        return false;
    }

    m_format = format;
    return InitInternal();
}

void DirectSoundAudio::Deinit()
{
    DeInitInternal();
}

bool DirectSoundAudio::Reinit(const AudioFormat& format)
{
    Deinit();
    return Init(format);
}

AudioFormat DirectSoundAudio::GetFormat() const
{
    return m_format;
}

bool DirectSoundAudio::Start()
{
#ifdef _WIN32
    if (!m_is_initialized)
    {
        m_error_message = "Audio not initialized";
        return false;
    }
    m_is_playing = true;
    return true;
#else
    m_error_message = "DirectSound not available on this platform";
    return false;
#endif
}

void DirectSoundAudio::Stop()
{
    m_is_playing = false;
}

bool DirectSoundAudio::IsPlaying() const
{
    return m_is_playing;
}

size_t DirectSoundAudio::Write(const void* data, size_t length)
{
#ifdef _WIN32
    if (!m_is_initialized || !data || length == 0)
        return 0;

    if (length > GetAvailableSpace())
        length = GetAvailableSpace();

    // Copy to internal buffer (simplified - in production would use DirectSound buffer directly)
    if (m_write_pos + length <= BUFFER_SIZE)
    {
        memcpy(&m_buffer[m_write_pos], data, length);
        m_write_pos = (m_write_pos + length) % BUFFER_SIZE;
    }
    else
    {
        size_t part1 = BUFFER_SIZE - m_write_pos;
        memcpy(&m_buffer[m_write_pos], data, part1);
        memcpy(&m_buffer[0], (const uint8_t*)data + part1, length - part1);
        m_write_pos = (length - part1) % BUFFER_SIZE;
    }

    return length;
#else
    return 0;
#endif
}

uint32_t DirectSoundAudio::GetPlayPosition() const
{
    return m_play_pos;
}

uint32_t DirectSoundAudio::GetWritePosition() const
{
    return m_write_pos;
}

size_t DirectSoundAudio::GetAvailableSpace() const
{
    if (m_write_pos >= m_play_pos)
        return BUFFER_SIZE - (m_write_pos - m_play_pos) - 1;
    else
        return (m_play_pos - m_write_pos) - 1;
}

const char* DirectSoundAudio::GetErrorMessage() const
{
    return m_error_message.c_str();
}

bool DirectSoundAudio::IsReady() const
{
    return m_is_initialized;
}

bool DirectSoundAudio::InitInternal()
{
#ifdef _WIN32
    DeInitInternal();

    // Create DirectSound device
    m_lpds = nullptr;
    if (DirectSoundCreate(NULL, (LPDIRECTSOUND*)&m_lpds, NULL) != DS_OK)
    {
        m_error_message = "Failed to create DirectSound device";
        return false;
    }

    // Create primary buffer descriptor
    DSBUFFERDESC dsbdesc;
    ZeroMemory(&dsbdesc, sizeof(DSBUFFERDESC));
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);
    dsbdesc.dwFlags = DSBCAPS_PRIMARYBUFFER;

    m_lpdsbPrimary = nullptr;
    LPDIRECTSOUND lpds = (LPDIRECTSOUND)m_lpds;
    if (lpds->CreateSoundBuffer(&dsbdesc, (LPDIRECTSOUNDBUFFER*)&m_lpdsbPrimary, NULL) != DS_OK)
    {
        m_error_message = "Failed to create primary sound buffer";
        lpds->Release();
        m_lpds = nullptr;
        return false;
    }

    // Set primary buffer format
    WAVEFORMATEX wfx;
    ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = m_format.channels;
    wfx.nSamplesPerSec = m_format.sample_rate;
    wfx.wBitsPerSample = m_format.bits_per_sample;
    wfx.nBlockAlign = m_format.block_align;
    wfx.nAvgBytesPerSec = m_format.avg_bytes_per_sec;
    wfx.cbSize = 0;

    LPDIRECTSOUNDBUFFER lpdsb = (LPDIRECTSOUNDBUFFER)m_lpdsbPrimary;
    if (lpdsb->SetFormat(&wfx) != DS_OK)
    {
        m_error_message = "Failed to set primary buffer format";
        lpdsb->Release();
        lpds->Release();
        m_lpds = nullptr;
        m_lpdsbPrimary = nullptr;
        return false;
    }

    m_is_initialized = true;
    m_error_message = "OK";
    return true;
#else
    m_error_message = "DirectSound not available on this platform";
    return false;
#endif
}

void DirectSoundAudio::DeInitInternal()
{
#ifdef _WIN32
    m_is_playing = false;

    if (m_lpdsbPrimary)
    {
        LPDIRECTSOUNDBUFFER lpdsb = (LPDIRECTSOUNDBUFFER)m_lpdsbPrimary;
        lpdsb->Release();
        m_lpdsbPrimary = nullptr;
    }

    if (m_lpds)
    {
        LPDIRECTSOUND lpds = (LPDIRECTSOUND)m_lpds;
        lpds->Release();
        m_lpds = nullptr;
    }

    m_is_initialized = false;
    memset(m_buffer, 0, sizeof(m_buffer));
    m_write_pos = 0;
    m_play_pos = 0;
#endif
}
