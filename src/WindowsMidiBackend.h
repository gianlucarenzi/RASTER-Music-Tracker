//
// WindowsMidiBackend.h - Windows MIDI API backend
//
// Implements IMidiBackend using Windows MIDI API (mmsystem.h)
//

#pragma once

#ifndef WINDOWSMIDIBACKEND_H
#define WINDOWSMIDIBACKEND_H

#include "MidiBackend.h"
#include <string>
#include <vector>

#ifdef _WIN32
    #include <windows.h>     // mmsystem.h needs the Windows base types (MinGW)
    #include <mmsystem.h>
#endif

/**
 * @brief Windows MIDI API backend
 * 
 * Uses Windows multimedia MIDI functions
 * Available only on Windows
 */
class WindowsMidiBackend : public IMidiBackend
{
public:
    WindowsMidiBackend();
    virtual ~WindowsMidiBackend();

    bool Init() override;
    void Deinit() override;
    bool Reinit() override;

    int GetPortCount() const override;
    std::string GetPortName(int portIndex) const override;
    bool OpenPort(int portIndex) override;
    void ClosePort() override;
    bool IsPortOpen() const override;

    bool SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) override;
    bool SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) override;
    bool SendControlChange(uint8_t channel, uint8_t controller, uint8_t value) override;
    bool SendProgramChange(uint8_t channel, uint8_t program) override;
    size_t SendRawMessage(const uint8_t* data, size_t length) override;

    bool PollMidiInput(MidiEvent& event) override;
    const char* GetErrorMessage() const override;
    bool IsReady() const override;

private:
    // Windows MIDI objects
#ifdef _WIN32
    HMIDIOUT m_hMidiOut;     ///< MIDI output device handle
    HMIDIIN m_hMidiIn;       ///< MIDI input device handle
#else
    void* m_hMidiOut;        // Dummy for non-Windows
    void* m_hMidiIn;         // Dummy for non-Windows
#endif

    int m_output_port_count;
    int m_input_port_count;
    int m_current_output_port;
    int m_current_input_port;
    bool m_is_initialized;
    std::string m_error_message;

    // Helper methods
    bool EnumeratePorts();
    void SetErrorMessage(const std::string& msg);

    // Cached port names
    std::vector<std::string> m_output_port_names;
    std::vector<std::string> m_input_port_names;
};

#endif // WINDOWSMIDIBACKEND_H
