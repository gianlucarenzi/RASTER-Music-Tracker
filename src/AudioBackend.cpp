//
// AudioBackend.cpp - Audio backend factory implementation
//

#include "AudioBackend.h"
#include "DirectSoundAudio.h"
#include "PortAudioBackend.h"

#ifdef _WIN32
    #define PREFER_DIRECTSOUND 1
#else
    #define PREFER_DIRECTSOUND 0
#endif

std::unique_ptr<IAudioBackend> AudioBackendFactory::Create()
{
    // Prefer native backend for each platform
#ifdef _WIN32
    return CreateDirectSound();
#else
    // On non-Windows, try PortAudio
    return CreatePortAudio();
#endif
}

std::unique_ptr<IAudioBackend> AudioBackendFactory::CreateDirectSound()
{
    return std::make_unique<DirectSoundAudio>();
}

std::unique_ptr<IAudioBackend> AudioBackendFactory::CreatePortAudio()
{
#ifdef _WIN32
    // Could fallback to DirectSound on Windows if PortAudio unavailable
    return std::make_unique<PortAudioBackend>();
#else
    // On non-Windows, use PortAudio
    return std::make_unique<PortAudioBackend>();
#endif
}
