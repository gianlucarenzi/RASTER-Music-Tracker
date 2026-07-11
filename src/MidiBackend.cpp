//
// MidiBackend.cpp - MIDI backend factory implementation
//

#include "MidiBackend.h"

#ifdef _WIN32
    #include "WindowsMidiBackend.h"
#endif

#include "RtMidiBackend.h"

std::unique_ptr<IMidiBackend> MidiBackendFactory::Create()
{
    // Prefer native backend for each platform
#ifdef _WIN32
    return CreateWindowsMidi();
#else
    // On non-Windows, use RtMidi
    return CreateRtMidi();
#endif
}

std::unique_ptr<IMidiBackend> MidiBackendFactory::CreateWindowsMidi()
{
#ifdef _WIN32
    return std::make_unique<WindowsMidiBackend>();
#else
    // Not available on non-Windows
    return nullptr;
#endif
}

std::unique_ptr<IMidiBackend> MidiBackendFactory::CreateRtMidi()
{
    return std::make_unique<RtMidiBackend>();
}
