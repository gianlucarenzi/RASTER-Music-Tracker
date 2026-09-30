//
// PortAudioBackend.cpp - PortAudio implementation
//

#include "PortAudioBackend.h"
#include <cstring>
#include <algorithm>

// PortAudio headers (only included in .cpp)
#ifdef __cplusplus
extern "C" {
#endif

#include <portaudio.h>

#ifdef __cplusplus
}
#endif

// Static instance for callback
static PortAudioBackend* g_portaudio_instance = nullptr;

// C-style callback wrapper that PortAudio will call
// This matches PortAudio's exact callback signature
extern "C" {
static int PortAudioCallbackWrapper(
    const void* input,
    void* output,
    unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData)
{
    // Call the static method implementation
    return PortAudioBackend::AudioCallbackImpl(input, output, frameCount,
                                               timeInfo, statusFlags, userData);
}
}

// PortAudio callback implementation - static method so it can access private members
int PortAudioBackend::AudioCallbackImpl(
    const void* input,
    void* output,
    unsigned long frameCount,
    const void* timeInfo,
    unsigned long statusFlags,
    void* userData)
{
    (void)input;
    (void)timeInfo;
    (void)statusFlags;

    PortAudioBackend* self = (PortAudioBackend*)userData;
    if (!self || !output)
        return paContinue;

    // Calculate bytes needed
    size_t bytes_needed = frameCount * self->m_format.block_align;
    uint8_t* out = (uint8_t*)output;

    // Read from circular buffer
    size_t read_pos = self->m_read_pos;
    size_t write_pos = self->m_write_pos;

    if (read_pos == write_pos) {
        // Buffer underrun - output silence
        memset(out, 0, bytes_needed);
    } else if (read_pos < write_pos) {
        // Simple case: data is contiguous
        size_t available = write_pos - read_pos;
        size_t to_copy = std::min(bytes_needed, available);
        memcpy(out, &self->m_buffer[read_pos], to_copy);
        self->m_read_pos = (read_pos + to_copy) % self->BUFFER_SIZE;

        // Pad with silence if not enough data
        if (to_copy < bytes_needed)
            memset(out + to_copy, 0, bytes_needed - to_copy);
    } else {
        // Wrap-around case
        size_t part1 = self->BUFFER_SIZE - read_pos;
        size_t to_copy = std::min(bytes_needed, part1);
        memcpy(out, &self->m_buffer[read_pos], to_copy);

        if (to_copy < bytes_needed) {
            size_t remaining = bytes_needed - to_copy;
            size_t part2 = std::min(remaining, write_pos);
            memcpy(out + to_copy, self->m_buffer, part2);
            to_copy += part2;

            // Pad with silence if still not enough
            if (to_copy < bytes_needed)
                memset(out + to_copy, 0, bytes_needed - to_copy);

            self->m_read_pos = part2;
        } else {
            self->m_read_pos = (read_pos + to_copy) % self->BUFFER_SIZE;
        }
    }

    return paContinue;
}

PortAudioBackend::PortAudioBackend()
    : m_is_playing(false), m_is_initialized(false), m_stream(nullptr),
      m_buffer(nullptr), m_buffer_capacity(BUFFER_SIZE),
      m_write_pos(0), m_read_pos(0)
{
    m_buffer = new uint8_t[BUFFER_SIZE];
    memset(m_buffer, 0, BUFFER_SIZE);
}

PortAudioBackend::~PortAudioBackend()
{
    Deinit();
    if (m_buffer) {
        delete[] m_buffer;
        m_buffer = nullptr;
    }
}

bool PortAudioBackend::Init(const AudioFormat& format)
{
    if (!format.IsValid()) {
        m_error_message = "Invalid audio format";
        return false;
    }

    m_format = format;
    return InitPortAudio();
}

void PortAudioBackend::Deinit()
{
    Stop();
    DeinitPortAudio();
    m_is_initialized = false;
}

bool PortAudioBackend::Reinit(const AudioFormat& format)
{
    Deinit();
    return Init(format);
}

AudioFormat PortAudioBackend::GetFormat() const
{
    return m_format;
}

bool PortAudioBackend::Start()
{
    if (!m_is_initialized) {
        m_error_message = "Audio not initialized";
        return false;
    }

    if (m_stream) {
        PaError err = Pa_StartStream((PaStream*)m_stream);
        if (err != paNoError) {
            m_error_message = std::string("PortAudio start error: ") + Pa_GetErrorText(err);
            return false;
        }
    }

    m_is_playing = true;
    return true;
}

void PortAudioBackend::Stop()
{
    m_is_playing = false;

    if (m_stream) {
        Pa_StopStream((PaStream*)m_stream);
    }
}

bool PortAudioBackend::IsPlaying() const
{
    return m_is_playing;
}

size_t PortAudioBackend::Write(const void* data, size_t length)
{
    if (!m_is_initialized || !data || length == 0)
        return 0;

    size_t available = GetAvailableSpace();
    if (length > available)
        length = available;

    // Write to circular buffer
    size_t write_pos = m_write_pos;
    const uint8_t* src = (const uint8_t*)data;

    if (write_pos + length <= BUFFER_SIZE) {
        // Simple case: no wrap-around
        memcpy(&m_buffer[write_pos], src, length);
    } else {
        // Wrap-around case
        size_t part1 = BUFFER_SIZE - write_pos;
        memcpy(&m_buffer[write_pos], src, part1);
        memcpy(&m_buffer[0], src + part1, length - part1);
    }

    m_write_pos = (write_pos + length) % BUFFER_SIZE;
    return length;
}

uint32_t PortAudioBackend::GetPlayPosition() const
{
    return (uint32_t)m_read_pos;
}

uint32_t PortAudioBackend::GetWritePosition() const
{
    return (uint32_t)m_write_pos;
}

size_t PortAudioBackend::GetAvailableSpace() const
{
    if (m_write_pos >= m_read_pos)
        return BUFFER_SIZE - (m_write_pos - m_read_pos) - 1;
    else
        return (m_read_pos - m_write_pos) - 1;
}

const char* PortAudioBackend::GetErrorMessage() const
{
    return m_error_message.c_str();
}

bool PortAudioBackend::IsReady() const
{
    return m_is_initialized && m_stream != nullptr;
}

bool PortAudioBackend::InitPortAudio()
{
    // Initialize PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        m_error_message = std::string("PortAudio init error: ") + Pa_GetErrorText(err);
        return false;
    }

    // Create output stream
    if (!CreateStream(m_format)) {
        Pa_Terminate();
        return false;
    }

    m_is_initialized = true;
    m_error_message = "OK";
    g_portaudio_instance = this;

    return true;
}

void PortAudioBackend::DeinitPortAudio()
{
    DestroyStream();

    PaError err = Pa_Terminate();
    if (err != paNoError) {
        m_error_message = std::string("PortAudio terminate error: ") + Pa_GetErrorText(err);
    }

    g_portaudio_instance = nullptr;
    m_is_initialized = false;
    m_write_pos = 0;
    m_read_pos = 0;
}

bool PortAudioBackend::CreateStream(const AudioFormat& format)
{
    PaStreamParameters outputParams;
    outputParams.device = Pa_GetDefaultOutputDevice();
    if (outputParams.device == paNoDevice) {
        m_error_message = "No default output device found";
        return false;
    }

    outputParams.channelCount = format.channels;
    outputParams.sampleFormat = GetPortAudioSampleFormat();
    outputParams.suggestedLatency = Pa_GetDeviceInfo(outputParams.device)->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;

    PaError err = Pa_OpenStream(
        (PaStream**)&m_stream,
        nullptr, // No input
        &outputParams,
        format.sample_rate,
        PORTAUDIO_FRAMES_PER_BUFFER,
        paClipOff,
        PortAudioCallbackWrapper,
        this);

    if (err != paNoError) {
        m_error_message = std::string("PortAudio open stream error: ") + Pa_GetErrorText(err);
        m_stream = nullptr;
        return false;
    }

    return true;
}

void PortAudioBackend::DestroyStream()
{
    if (m_stream) {
        Pa_CloseStream((PaStream*)m_stream);
        m_stream = nullptr;
    }
}

int PortAudioBackend::GetPortAudioSampleFormat() const
{
    if (m_format.bits_per_sample == 16)
        return paInt16;
    else if (m_format.bits_per_sample == 32)
        return paInt32;
    else
        return paInt16; // Default to 16-bit
}

size_t PortAudioBackend::GetAvailableSpaceUnsafe() const
{
    if (m_write_pos >= m_read_pos)
        return BUFFER_SIZE - (m_write_pos - m_read_pos) - 1;
    else
        return (m_read_pos - m_write_pos) - 1;
}

size_t PortAudioBackend::GetAvailableDataUnsafe() const
{
    if (m_write_pos >= m_read_pos)
        return m_write_pos - m_read_pos;
    else
        return BUFFER_SIZE - (m_read_pos - m_write_pos);
}
