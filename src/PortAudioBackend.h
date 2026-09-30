//
// PortAudioBackend.h - PortAudio cross-platform backend
//
// Implements IAudioBackend using PortAudio library
// Available on: Windows, Linux, macOS
//

#pragma once

#ifndef PORTAUDIOBACKEND_H
#define PORTAUDIOBACKEND_H

#include "AudioBackend.h"
#include <string>

// Forward declaration to avoid including portaudio.h in header
typedef void PaStream;

/**
 * @brief PortAudio backend implementation
 * 
 * Provides cross-platform audio I/O via PortAudio.
 * Works on Windows, Linux, and macOS.
 * 
 * Benefits:
 * - Same API across all platforms
 * - ALSA/PulseAudio on Linux
 * - WASAPI on modern Windows
 * - CoreAudio on macOS
 */
class PortAudioBackend : public IAudioBackend {
public:
    static constexpr size_t BUFFER_SIZE = 0x8000;               ///< Circular buffer size
    static constexpr size_t PORTAUDIO_FRAMES_PER_BUFFER = 1024; ///< PortAudio buffer

    PortAudioBackend();
    virtual ~PortAudioBackend();

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

    // PortAudio callback - static method with C linkage wrapper
    // Made public so external wrapper can call it
    static int AudioCallbackImpl(
        const void* input,
        void* output,
        unsigned long frameCount,
        const void* timeInfo,
        unsigned long statusFlags,
        void* userData);

private:
    AudioFormat m_format;
    std::string m_error_message;
    bool m_is_playing;
    bool m_is_initialized;

    // PortAudio stream
    PaStream* m_stream;

    // Circular buffer management
    uint8_t* m_buffer;
    size_t m_buffer_capacity;
    volatile size_t m_write_pos;
    volatile size_t m_read_pos;

    // Initialization helpers
    bool InitPortAudio();
    void DeinitPortAudio();
    bool CreateStream(const AudioFormat& format);
    void DestroyStream();

    // Helper to convert format to PortAudio sample format
    int GetPortAudioSampleFormat() const;

    // Thread-safe buffer operations
    size_t GetAvailableSpaceUnsafe() const;
    size_t GetAvailableDataUnsafe() const;
};

#endif // PORTAUDIOBACKEND_H
