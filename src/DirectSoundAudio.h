//
// DirectSoundAudio.h - DirectSound backend for Windows
//
// Implements IAudioBackend using Windows DirectSound API
//

#pragma once

#ifndef DIRECTSOUNDAUDIO_H
#define DIRECTSOUNDAUDIO_H

#include "AudioBackend.h"
#include <string>

#ifdef _WIN32
#include <dsound.h>
#endif

/**
 * @brief DirectSound audio backend (Windows only)
 * 
 * Wraps the existing PokeyRenderer DirectSound code
 */
class DirectSoundAudio : public IAudioBackend {
public:
    static constexpr size_t BUFFER_SIZE = 0x8000; // Must be a power of 2

    DirectSoundAudio();
    virtual ~DirectSoundAudio();

    bool Init(const AudioFormat& format) override;
    void Deinit() override;
    bool Reinit(const AudioFormat& format) override;

    AudioFormat GetFormat() const override;
    bool Start() override;
    void Stop() override;
    bool IsPlaying() const override;

    size_t Write(const void* data, size_t length) override;
    uint32_t GetPlayPosition() const override;
    uint32_t GetWritePosition() const override;
    size_t GetAvailableSpace() const override;
    size_t GetBufferSize() const override { return BUFFER_SIZE; }

    const char* GetErrorMessage() const override;
    bool IsReady() const override;

private:
    // Windows DirectSound objects
#ifdef _WIN32
    LPDIRECTSOUND m_lpds;               ///< DirectSound device
    LPDIRECTSOUNDBUFFER m_lpdsbPrimary; ///< Primary sound buffer
#else
    void* m_lpds;         // Dummy for non-Windows
    void* m_lpdsbPrimary; // Dummy for non-Windows
#endif

    AudioFormat m_format;
    std::string m_error_message;
    bool m_is_playing;
    bool m_is_initialized;

    // State
    uint32_t m_write_pos;
    uint32_t m_play_pos;
    uint8_t m_buffer[BUFFER_SIZE];

    // Internal helpers
    bool InitInternal();
    void DeInitInternal();
};

#endif // DIRECTSOUNDAUDIO_H
