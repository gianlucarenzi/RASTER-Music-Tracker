// RmtMidiRt.cpp - MIDI IN of CRmtMidi (RmtMidi.h) without the Windows MIDI API
//
// Builds without MFC (Qt frontend, core-only) use this instead of RmtMidi.cpp:
// the MIDI IN devices are the RtMidi input ports (the ALSA sequencer on
// Linux), and each message goes to CSong::MidiEvent() packed like the
// dwParam1 of a MIM_DATA message, from the RtMidi thread, as the MidiInProc()
// callback of RmtMidi.cpp does from the winmm one. midiInGetNumDevs() and
// midiInGetDevCaps() (MfcTypes.h) list the same ports for the configuration
// code, which finds the device by its name. Without RtMidi (RTMIDI_AVAILABLE
// not defined) there are no devices.

#include "PlatformTypes.h"
#include "RmtMidi.h"
#include "Song.h"
#include "Global.h"

#ifdef RTMIDI_AVAILABLE
#include <RtMidi.h>
#endif

#include <string>
#include <vector>

extern CSong g_Song;

#ifdef RTMIDI_AVAILABLE

// The names of the input ports. ALSA ones end with the client:port numbers
// ("Midi Through:Midi Through Port-0 14:0"), which may change from a session
// to the next: they are left out, so the name saved in the configuration
// still finds the device
static std::vector<std::string> InputPortNames()
{
    std::vector<std::string> names;
    try {
        RtMidiIn in(RtMidi::UNSPECIFIED, "RMT");
        unsigned int count = in.getPortCount();
        for (unsigned int i = 0; i < count; i++) {
            std::string name = in.getPortName(i);
            size_t space = name.find_last_of(' ');
            if (space != std::string::npos && space + 1 < name.size()
                && name.find_first_not_of("0123456789:", space + 1) == std::string::npos
                && name.find(':', space + 1) != std::string::npos)
                name.erase(space);
            names.push_back(name);
        }
    }
    catch (const RtMidiError&) {
    }
    return names;
}

// RtMidi thread: the message as the DWORD of MIM_DATA (status, data1, data2)
static void MidiInCallback(double, std::vector<unsigned char>* message, void*)
{
    if (!message || message->empty() || message->size() > 3) return;
    DWORD data = 0;
    for (size_t i = 0; i < message->size(); i++) data |= (DWORD)(*message)[i] << (8 * i);
    g_Song.MidiEvent(data);
}

#else

static std::vector<std::string> InputPortNames() { return {}; }

#endif

UINT midiInGetNumDevs()
{
    return (UINT)InputPortNames().size();
}

UINT midiInGetDevCaps(UINT_PTR id, MIDIINCAPS* caps, UINT)
{
    std::vector<std::string> names = InputPortNames();
    if (!caps || id >= names.size()) return MMSYSERR_BADDEVICEID;
    *caps = {};
    strncpy(caps->szPname, names[id].c_str(), sizeof(caps->szPname) - 1);
    return MMSYSERR_NOERROR;
}

// Same defaults as RmtMidi.cpp
CRmtMidi::CRmtMidi()
{
    m_MidiIsOn = 0;
    m_MidiInHandle = NULL;
    m_MidiInDeviceId = -1;
    m_MidiInDeviceName[0] = 0;

    m_TouchResponse = 0;
    m_VolumeOffset = 1;
    m_NoteOff = 0;

    for (int i = 0; i < 16; i++) {
        m_LastNoteOnChannel[i] = -1;
        m_NoteVolumeOnChannel[i] = 0;
        m_InstrumentOnChannel[i] = 0;
    }
}

CRmtMidi::~CRmtMidi()
{
    MidiOff();
}

// CRmtMidi::MidiInit() of RmtMidi.cpp: the device of the configured name
int CRmtMidi::MidiInit()
{
    int wasOnOff = IsOn();

    MidiOff();

    if (m_MidiInDeviceName[0] == 0)
    {
        m_MidiInDeviceId = -1;
        return 1;	//does not want a MIDI device
    }

    std::vector<std::string> names = InputPortNames();
    for (size_t i = 0; i < names.size(); i++)
    {
        if (names[i] == m_MidiInDeviceName)
        {
            m_MidiInDeviceId = (int)i;
            if (wasOnOff) MidiOn();
            return 1;
        }
    }
    m_MidiInDeviceId = -1;

    MessageBox(g_hwnd, CString("Can't init the MIDI IN device\n") + m_MidiInDeviceName, "MIDI IN error", MB_ICONEXCLAMATION);

    m_MidiInDeviceName[0] = 0;

    return 0;
}

int CRmtMidi::MidiOn()
{
    // Init the MIDI channel buffers
    for (int i = 0; i < 16; i++)
    {
        m_LastNoteOnChannel[i] = -1;	// Last pressed keys on each channel
        m_NoteVolumeOnChannel[i] = 0;	// Volume
        m_InstrumentOnChannel[i] = 0;	// Instrument numbers
    }

    if (m_MidiInDeviceId < 0) return 0;
    if (IsOn()) MidiOff();

#ifdef RTMIDI_AVAILABLE
    RtMidiIn* in = nullptr;
    try {
        in = new RtMidiIn(RtMidi::UNSPECIFIED, "RMT");
        in->openPort(m_MidiInDeviceId, "MIDI IN");
        in->ignoreTypes(true, true, true);			// no SysEx, timing or active sensing
        in->setCallback(MidiInCallback);
        m_MidiInHandle = in;
        m_MidiIsOn = 1;
        return 1;
    }
    catch (const RtMidiError&) {
        delete in;
    }
#endif
    MessageBox(0, "Can't open selected MIDI IN device.", "MidiInOpen error", MB_ICONERROR);
    return 0;
}

void CRmtMidi::MidiOff()
{
#ifdef RTMIDI_AVAILABLE
    // Deleting the RtMidiIn waits for its thread: no callback runs after it
    delete static_cast<RtMidiIn*>(m_MidiInHandle);
#endif
    m_MidiInHandle = NULL;
    m_MidiIsOn = 0;
}

int CRmtMidi::MidiRestart()
{
    MidiOff();
    return MidiOn();
}
