//
// RtMidiBackend.cpp - RtMidi implementation
//

#include "RtMidiBackend.h"
#include <cstring>

#ifdef RTMIDI_AVAILABLE
#include <RtMidi.h>
#endif

RtMidiBackend::RtMidiBackend()
    : m_midiOut(nullptr), m_midiIn(nullptr),
      m_port_count(0), m_current_port(-1),
      m_is_initialized(false)
{
}

RtMidiBackend::~RtMidiBackend()
{
    Deinit();
}

bool RtMidiBackend::Init()
{
#ifdef RTMIDI_AVAILABLE
    return InitRtMidi();
#else
    m_error_message = "RtMidi not available - install librtmidi-dev";
    return false;
#endif
}

void RtMidiBackend::Deinit()
{
    ClosePort();
    DeinitRtMidi();
}

bool RtMidiBackend::Reinit()
{
    Deinit();
    return Init();
}

#ifdef RTMIDI_AVAILABLE

bool RtMidiBackend::InitRtMidi()
{
    if (m_is_initialized)
        return true;

    try {
        // Create RtMidiOut and RtMidiIn objects
        m_midiOut = new RtMidiOut();
        m_midiIn = new RtMidiIn();

        // Get port count
        m_port_count = static_cast<int>(((RtMidiOut*)m_midiOut)->getPortCount());

        // Enumerate port names
        if (!EnumeratePorts()) {
            m_error_message = "Failed to enumerate MIDI ports";
            DeinitRtMidi();
            return false;
        }

        m_error_message = "OK";
        m_is_initialized = true;
        return true;
    } catch (const RtMidiError& e) {
        m_error_message = std::string("RtMidi init error: ") + e.what();
        return false;
    } catch (const std::exception& e) {
        m_error_message = std::string("RtMidi init exception: ") + e.what();
        return false;
    }
}

void RtMidiBackend::DeinitRtMidi()
{
#ifdef RTMIDI_AVAILABLE
    if (m_midiOut) {
        try {
            RtMidiOut* out = static_cast<RtMidiOut*>(m_midiOut);
            if (out->isPortOpen())
                out->closePort();
            delete out;
        } catch (...) {
        }
        m_midiOut = nullptr;
    }

    if (m_midiIn) {
        try {
            RtMidiIn* in = static_cast<RtMidiIn*>(m_midiIn);
            if (in->isPortOpen())
                in->closePort();
            delete in;
        } catch (...) {
        }
        m_midiIn = nullptr;
    }
#endif

    m_is_initialized = false;
    m_current_port = -1;
    m_port_names.clear();
}

#else // !RTMIDI_AVAILABLE

// Stub implementations for when RtMidi is not available
bool RtMidiBackend::InitRtMidi()
{
    m_error_message = "RtMidi not compiled in";
    return false;
}

void RtMidiBackend::DeinitRtMidi()
{
}

#endif // RTMIDI_AVAILABLE

int RtMidiBackend::GetPortCount() const
{
    return m_port_count;
}

std::string RtMidiBackend::GetPortName(int portIndex) const
{
    if (portIndex < 0 || portIndex >= (int)m_port_names.size())
        return "";
    return m_port_names[portIndex];
}

bool RtMidiBackend::OpenPort(int portIndex)
{
    if (!m_midiOut) {
        m_error_message = "RtMidiOut not initialized";
        return false;
    }

    if (portIndex < 0 || portIndex >= m_port_count) {
        m_error_message = "Invalid port index";
        return false;
    }

    if (m_current_port >= 0)
        ClosePort();

#ifdef RTMIDI_AVAILABLE
    try {
        RtMidiOut* out = static_cast<RtMidiOut*>(m_midiOut);
        out->openPort(portIndex);
        m_current_port = portIndex;
        m_error_message = "OK";
        return true;
    } catch (const RtMidiError& e) {
        m_error_message = std::string("Failed to open MIDI port: ") + e.what();
        return false;
    }
#else
    return false;
#endif
}

void RtMidiBackend::ClosePort()
{
#ifdef RTMIDI_AVAILABLE
    if (m_midiOut && m_current_port >= 0) {
        try {
            RtMidiOut* out = static_cast<RtMidiOut*>(m_midiOut);
            if (out->isPortOpen())
                out->closePort();
        } catch (...) {
        }
    }
#endif
    m_current_port = -1;
}

bool RtMidiBackend::IsPortOpen() const
{
    return m_current_port >= 0;
}

bool RtMidiBackend::SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity)
{
    if (!m_midiOut || !IsPortOpen()) {
        m_error_message = "No MIDI port open";
        return false;
    }

#ifdef RTMIDI_AVAILABLE
    try {
        unsigned char message[3] = {
            (unsigned char)(0x90 | (channel & 0x0F)),
            (unsigned char)(note & 0x7F),
            (unsigned char)(velocity & 0x7F)
        };

        RtMidiOut* out = static_cast<RtMidiOut*>(m_midiOut);
        out->sendMessage(message, 3);
        return true;
    } catch (const RtMidiError& e) {
        m_error_message = std::string("Error sending NoteOn: ") + e.what();
        return false;
    }
#else
    return false;
#endif
}

bool RtMidiBackend::SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity)
{
    if (!m_midiOut || !IsPortOpen()) {
        m_error_message = "No MIDI port open";
        return false;
    }

#ifdef RTMIDI_AVAILABLE
    try {
        unsigned char message[3] = {
            (unsigned char)(0x80 | (channel & 0x0F)),
            (unsigned char)(note & 0x7F),
            (unsigned char)(velocity & 0x7F)
        };

        RtMidiOut* out = static_cast<RtMidiOut*>(m_midiOut);
        out->sendMessage(message, 3);
        return true;
    } catch (const RtMidiError& e) {
        m_error_message = std::string("Error sending NoteOff: ") + e.what();
        return false;
    }
#else
    return false;
#endif
}

bool RtMidiBackend::SendControlChange(uint8_t channel, uint8_t controller, uint8_t value)
{
    if (!m_midiOut || !IsPortOpen()) {
        m_error_message = "No MIDI port open";
        return false;
    }

#ifdef RTMIDI_AVAILABLE
    try {
        unsigned char message[3] = {
            (unsigned char)(0xB0 | (channel & 0x0F)),
            (unsigned char)(controller & 0x7F),
            (unsigned char)(value & 0x7F)
        };

        RtMidiOut* out = static_cast<RtMidiOut*>(m_midiOut);
        out->sendMessage(message, 3);
        return true;
    } catch (const RtMidiError& e) {
        m_error_message = std::string("Error sending CC: ") + e.what();
        return false;
    }
#else
    return false;
#endif
}

bool RtMidiBackend::SendProgramChange(uint8_t channel, uint8_t program)
{
    if (!m_midiOut || !IsPortOpen()) {
        m_error_message = "No MIDI port open";
        return false;
    }

#ifdef RTMIDI_AVAILABLE
    try {
        unsigned char message[2] = {
            (unsigned char)(0xC0 | (channel & 0x0F)),
            (unsigned char)(program & 0x7F)
        };

        RtMidiOut* out = static_cast<RtMidiOut*>(m_midiOut);
        out->sendMessage(message, 2);
        return true;
    } catch (const RtMidiError& e) {
        m_error_message = std::string("Error sending PC: ") + e.what();
        return false;
    }
#else
    return false;
#endif
}

size_t RtMidiBackend::SendRawMessage(const uint8_t* data, size_t length)
{
    if (!m_midiOut || !IsPortOpen() || !data || length == 0)
        return 0;

#ifdef RTMIDI_AVAILABLE
    try {
        RtMidiOut* out = static_cast<RtMidiOut*>(m_midiOut);
        out->sendMessage(data, length);
        return length;
    } catch (const RtMidiError& e) {
        m_error_message = std::string("Error sending raw message: ") + e.what();
        return 0;
    }
#else
    return 0;
#endif
}

bool RtMidiBackend::PollMidiInput(MidiEvent& event)
{
    if (!m_midiIn || !IsPortOpen())
        return false;

#ifdef RTMIDI_AVAILABLE
    try {
        RtMidiIn* in = static_cast<RtMidiIn*>(m_midiIn);
        std::vector<unsigned char> message;
        double stamp = in->getMessage(&message);

        if (message.size() > 0) {
            // Parse MIDI message
            event.timestamp = static_cast<uint32_t>(stamp * 1000); // Convert to ms

            unsigned char status = message[0];
            event.type = MidiMessageType::Unknown;
            event.channel = status & 0x0F;

            // Determine message type
            if ((status & 0xF0) == 0x90 && message.size() >= 3) {
                event.type = MidiMessageType::NoteOn;
                event.data1 = message[1]; // Note
                event.data2 = message[2]; // Velocity
            } else if ((status & 0xF0) == 0x80 && message.size() >= 3) {
                event.type = MidiMessageType::NoteOff;
                event.data1 = message[1]; // Note
                event.data2 = message[2]; // Velocity
            } else if ((status & 0xF0) == 0xB0 && message.size() >= 3) {
                event.type = MidiMessageType::ControlChange;
                event.data1 = message[1]; // Controller
                event.data2 = message[2]; // Value
            } else if ((status & 0xF0) == 0xC0 && message.size() >= 2) {
                event.type = MidiMessageType::ProgramChange;
                event.data1 = message[1]; // Program
                event.data2 = 0;
            }

            return true;
        }
    } catch (const RtMidiError& e) {
        m_error_message = std::string("Error polling MIDI input: ") + e.what();
    }
#endif

    return false;
}

const char* RtMidiBackend::GetErrorMessage() const
{
    return m_error_message.c_str();
}

bool RtMidiBackend::IsReady() const
{
    return m_is_initialized && IsPortOpen();
}

bool RtMidiBackend::EnumeratePorts()
{
    m_port_names.clear();

#ifdef RTMIDI_AVAILABLE
    try {
        RtMidiOut* out = static_cast<RtMidiOut*>(m_midiOut);
        for (int i = 0; i < m_port_count; i++) {
            try {
                std::string portName = out->getPortName(i);
                if (!portName.empty())
                    m_port_names.push_back(portName);
                else
                    m_port_names.push_back("Unknown Port");
            } catch (...) {
                m_port_names.push_back("Unknown Port");
            }
        }
    } catch (const RtMidiError& e) {
        m_error_message = std::string("Error enumerating ports: ") + e.what();
        return false;
    }
#else
    m_port_names.push_back("(RtMidi disabled)");
#endif

    return true;
}
