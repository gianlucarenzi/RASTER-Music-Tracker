//
// WindowsMidiBackend.cpp - Windows MIDI API implementation
//

#include "WindowsMidiBackend.h"
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")
#endif

WindowsMidiBackend::WindowsMidiBackend()
    : m_output_port_count(0), m_input_port_count(0),
      m_current_output_port(-1), m_current_input_port(-1),
      m_is_initialized(false)
#ifdef _WIN32
      ,
      m_hMidiOut(nullptr), m_hMidiIn(nullptr)
#endif
{
}

WindowsMidiBackend::~WindowsMidiBackend()
{
    Deinit();
}

bool WindowsMidiBackend::Init()
{
#ifdef _WIN32
    // Enumerate available ports
    m_output_port_count = midiOutGetNumDevs();
    m_input_port_count = midiInGetNumDevs();

    if (m_output_port_count == 0) {
        SetErrorMessage("No MIDI output ports available");
        return false;
    }

    m_error_message = "OK";
    m_is_initialized = true;
    return true;
#else
    SetErrorMessage("Windows MIDI API not available on this platform");
    return false;
#endif
}

void WindowsMidiBackend::Deinit()
{
    ClosePort();
#ifdef _WIN32
    if (m_hMidiOut != nullptr) {
        midiOutClose((HMIDIOUT)m_hMidiOut);
        m_hMidiOut = nullptr;
    }
    if (m_hMidiIn != nullptr) {
        midiInClose((HMIDIIN)m_hMidiIn);
        m_hMidiIn = nullptr;
    }
#endif
    m_is_initialized = false;
    m_current_output_port = -1;
    m_current_input_port = -1;
}

bool WindowsMidiBackend::Reinit()
{
    Deinit();
    return Init();
}

int WindowsMidiBackend::GetPortCount() const
{
    return m_output_port_count;
}

std::string WindowsMidiBackend::GetPortName(int portIndex) const
{
#ifdef _WIN32
    if (portIndex < 0 || portIndex >= m_output_port_count)
        return "";

    MIDIOUTCAPSA caps;
    MMRESULT result = midiOutGetDevCapsA(portIndex, &caps, sizeof(MIDIOUTCAPSA));
    if (result == MMSYSERR_NOERROR)
        return std::string(caps.szPname);
    else
        return "";
#else
    return "";
#endif
}

bool WindowsMidiBackend::OpenPort(int portIndex)
{
#ifdef _WIN32
    if (portIndex < 0 || portIndex >= m_output_port_count) {
        SetErrorMessage("Invalid port index");
        return false;
    }

    if (m_hMidiOut != nullptr)
        ClosePort();

    MMRESULT result = midiOutOpen((LPHMIDIOUT)&m_hMidiOut, portIndex, 0, 0, CALLBACK_NULL);
    if (result != MMSYSERR_NOERROR) {
        SetErrorMessage("Failed to open MIDI output port");
        m_hMidiOut = nullptr;
        return false;
    }

    m_current_output_port = portIndex;
    m_error_message = "OK";
    return true;
#else
    SetErrorMessage("Windows MIDI API not available");
    return false;
#endif
}

void WindowsMidiBackend::ClosePort()
{
#ifdef _WIN32
    if (m_hMidiOut != nullptr) {
        midiOutClose((HMIDIOUT)m_hMidiOut);
        m_hMidiOut = nullptr;
    }
#endif
    m_current_output_port = -1;
}

bool WindowsMidiBackend::IsPortOpen() const
{
    return m_current_output_port >= 0;
}

bool WindowsMidiBackend::SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
#ifdef _WIN32
    if (m_hMidiOut == nullptr) {
        SetErrorMessage("No MIDI port open");
        return false;
    }

    // Construct MIDI message: Status byte | channel, note, velocity
    uint32_t message = ((0x90 | channel) & 0xFF) |
                       ((note & 0x7F) << 8) |
                       ((velocity & 0x7F) << 16);

    MMRESULT result = midiOutShortMsg((HMIDIOUT)m_hMidiOut, message);
    return result == MMSYSERR_NOERROR;
#else
    return false;
#endif
}

bool WindowsMidiBackend::SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
#ifdef _WIN32
    if (m_hMidiOut == nullptr) {
        SetErrorMessage("No MIDI port open");
        return false;
    }

    // Construct MIDI message: Status byte | channel, note, velocity
    uint32_t message = ((0x80 | channel) & 0xFF) |
                       ((note & 0x7F) << 8) |
                       ((velocity & 0x7F) << 16);

    MMRESULT result = midiOutShortMsg((HMIDIOUT)m_hMidiOut, message);
    return result == MMSYSERR_NOERROR;
#else
    return false;
#endif
}

bool WindowsMidiBackend::SendControlChange(uint8_t channel, uint8_t controller, uint8_t value)
{
#ifdef _WIN32
    if (m_hMidiOut == nullptr) {
        SetErrorMessage("No MIDI port open");
        return false;
    }

    // Construct MIDI CC message: 0xB0 | channel, controller, value
    uint32_t message = ((0xB0 | channel) & 0xFF) |
                       ((controller & 0x7F) << 8) |
                       ((value & 0x7F) << 16);

    MMRESULT result = midiOutShortMsg((HMIDIOUT)m_hMidiOut, message);
    return result == MMSYSERR_NOERROR;
#else
    return false;
#endif
}

bool WindowsMidiBackend::SendProgramChange(uint8_t channel, uint8_t program)
{
#ifdef _WIN32
    if (m_hMidiOut == nullptr) {
        SetErrorMessage("No MIDI port open");
        return false;
    }

    // Construct MIDI program change message: 0xC0 | channel, program
    uint32_t message = ((0xC0 | channel) & 0xFF) |
                       ((program & 0x7F) << 8);

    MMRESULT result = midiOutShortMsg((HMIDIOUT)m_hMidiOut, message);
    return result == MMSYSERR_NOERROR;
#else
    return false;
#endif
}

size_t WindowsMidiBackend::SendRawMessage(const uint8_t* data, size_t length)
{
#ifdef _WIN32
    if (m_hMidiOut == nullptr || !data || length == 0)
        return 0;

    // For short messages (3 bytes or less)
    if (length <= 3) {
        uint32_t message = 0;
        for (size_t i = 0; i < length; i++)
            message |= (data[i] << (i * 8));

        MMRESULT result = midiOutShortMsg((HMIDIOUT)m_hMidiOut, message);
        return (result == MMSYSERR_NOERROR) ? length : 0;
    }

    // For longer messages (system exclusive), would need midiOutLongMsg
    // For now, just support short messages
    SetErrorMessage("Long MIDI messages not yet implemented");
    return 0;
#else
    return 0;
#endif
}

bool WindowsMidiBackend::PollMidiInput(MidiEvent& event)
{
    // MIDI input polling not yet implemented
    // This would require proper thread-safe circular buffer for input
    return false;
}

const char* WindowsMidiBackend::GetErrorMessage() const
{
    return m_error_message.c_str();
}

bool WindowsMidiBackend::IsReady() const
{
    return m_is_initialized && IsPortOpen();
}

void WindowsMidiBackend::SetErrorMessage(const std::string& msg)
{
    m_error_message = msg;
}
