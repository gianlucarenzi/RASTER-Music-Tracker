//
// AudioBackend.h - Cross-platform audio abstraction layer
//
// Decouples RMT from Windows-only DirectSound
// Allows multiple backends: DirectSound (Windows), PortAudio (cross-platform)
//

#pragma once

#ifndef AUDIOBACKEND_H
#define AUDIOBACKEND_H

#include <cstdint>
#include <memory>

// Forward declarations
// Same declaration as <mmsystem.h> (a plain "struct WAVEFORMATEX;" clashes
// with its typedef under MinGW)
struct tWAVEFORMATEX;
typedef struct tWAVEFORMATEX WAVEFORMATEX;

/**
 * @brief Audio format descriptor (cross-platform compatible)
 */
struct AudioFormat {
    uint16_t channels;          ///< 1 = mono, 2 = stereo
    uint32_t sample_rate;       ///< Samples per second (44100, 48000, etc)
    uint16_t bits_per_sample;   ///< 8 or 16
    uint16_t block_align;       ///< Bytes per sample frame
    uint32_t avg_bytes_per_sec; ///< Sample rate * block align

    AudioFormat() : channels(0), sample_rate(0), bits_per_sample(0),
                    block_align(0), avg_bytes_per_sec(0) {}

    AudioFormat(uint16_t ch, uint32_t sr, uint16_t bps)
        : channels(ch), sample_rate(sr), bits_per_sample(bps)
    {
        block_align = (bits_per_sample / 8) * channels;
        avg_bytes_per_sec = sample_rate * block_align;
    }

    bool IsValid() const
    {
        return channels > 0 && sample_rate > 0 && bits_per_sample > 0;
    }
};

/**
 * @brief Abstract audio backend interface
 * 
 * Implementations must provide:
 * - Initialization/deinitialization
 * - Audio format setup
 * - Buffer management
 * - Audio output
 */
class IAudioBackend {
public:
    virtual ~IAudioBackend() = default;

    /**
     * Initialize audio backend
     * @param format Desired audio format
     * @return true if successful
     */
    virtual bool Init(const AudioFormat& format) = 0;

    /**
     * Deinitialize and clean up audio resources
     */
    virtual void Deinit() = 0;

    /**
     * Reinitialize with new audio format
     * @param format New audio format
     * @return true if successful
     */
    virtual bool Reinit(const AudioFormat& format) = 0;

    /**
     * Get current audio format
     * @return Current audio format
     */
    virtual AudioFormat GetFormat() const = 0;

    /**
     * Start audio playback
     * @return true if successful
     */
    virtual bool Start() = 0;

    /**
     * Stop audio playback
     */
    virtual void Stop() = 0;

    /**
     * Check if audio is currently playing
     * @return true if playing
     */
    virtual bool IsPlaying() const = 0;

    /**
     * Write audio samples to output buffer
     * @param data Pointer to audio data
     * @param length Number of bytes to write
     * @return Number of bytes actually written
     */
    virtual size_t Write(const void* data, size_t length) = 0;

    /**
     * Get current playback position in bytes
     * @return Current position
     */
    virtual uint32_t GetPlayPosition() const = 0;

    /**
     * Get current write position in bytes
     * @return Write position
     */
    virtual uint32_t GetWritePosition() const = 0;

    /**
     * Get number of bytes currently available in buffer
     * @return Available bytes
     */
    virtual size_t GetAvailableSpace() const = 0;

    /**
     * Get total buffer size in bytes
     * @return Buffer size
     */
    virtual size_t GetBufferSize() const = 0;

    /**
     * Get error message if initialization failed
     * @return Error string
     */
    virtual const char* GetErrorMessage() const = 0;

    /**
     * Check if backend is ready
     * @return true if initialized and ready
     */
    virtual bool IsReady() const = 0;
};

/**
 * @brief Audio backend factory
 * 
 * Creates appropriate backend for current platform
 */
class AudioBackendFactory {
public:
    /**
     * Create native audio backend for current platform
     * @return Pointer to new AudioBackend instance (caller owns)
     */
    static std::unique_ptr<IAudioBackend> Create();

    /**
     * Create DirectSound backend (Windows only)
     * @return Pointer to new DirectSoundAudio instance (caller owns)
     */
    static std::unique_ptr<IAudioBackend> CreateDirectSound();

    /**
     * Create PortAudio backend (cross-platform)
     * @return Pointer to new PortAudioBackend instance (caller owns)
     */
    static std::unique_ptr<IAudioBackend> CreatePortAudio();
};

#endif // AUDIOBACKEND_H
