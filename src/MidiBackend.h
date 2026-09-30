//
// MidiBackend.h - Cross-platform MIDI abstraction layer
//
// Decouples RMT from Windows-only MIDI API
// Allows multiple backends: Windows MIDI API (Windows), RtMidi (cross-platform)
//

#pragma once

#ifndef MIDIBACKEND_H
#define MIDIBACKEND_H

#include <cstdint>
#include <memory>
#include <string>

/**
 * @brief MIDI message types
 */
enum class MidiMessageType {
    NoteOff = 0x80,
    NoteOn = 0x90,
    PolyKeyPressure = 0xA0,
    ControlChange = 0xB0,
    ProgramChange = 0xC0,
    ChannelPressure = 0xD0,
    PitchBend = 0xE0,
    SystemExclusive = 0xF0,
    Unknown = 0xFF
};

/**
 * @brief MIDI event descriptor
 */
struct MidiEvent {
    MidiMessageType type;
    uint8_t channel;    // 0-15 (MIDI channels 1-16)
    uint8_t data1;      // Note number, CC, program, etc.
    uint8_t data2;      // Velocity, CC value, etc.
    uint32_t timestamp; // Milliseconds since start

    MidiEvent() : type(MidiMessageType::Unknown), channel(0), data1(0), data2(0), timestamp(0) {}

    MidiEvent(MidiMessageType t, uint8_t ch, uint8_t d1, uint8_t d2)
        : type(t), channel(ch), data1(d1), data2(d2), timestamp(0) {}
};

/**
 * @brief Abstract MIDI backend interface
 * 
 * Implementations must provide:
 * - Port enumeration
 * - Open/close operations
 * - MIDI message transmission
 * - MIDI message reception
 */
class IMidiBackend {
public:
    virtual ~IMidiBackend() = default;

    /**
     * Initialize MIDI system
     * @return true on success
     */
    virtual bool Init() = 0;

    /**
     * Clean up resources
     */
    virtual void Deinit() = 0;

    /**
     * Reinitialize the MIDI system
     * @return true on success
     */
    virtual bool Reinit() = 0;

    /**
     * Enumerate available MIDI output ports
     * @return Number of ports available
     */
    virtual int GetPortCount() const = 0;

    /**
     * Get name of MIDI output port
     * @param portIndex Port index (0-based)
     * @return Port name or empty string if invalid
     */
    virtual std::string GetPortName(int portIndex) const = 0;

    /**
     * Open a MIDI output port
     * @param portIndex Port index (0-based)
     * @return true on success
     */
    virtual bool OpenPort(int portIndex) = 0;

    /**
     * Close the current MIDI port
     */
    virtual void ClosePort() = 0;

    /**
     * Check if a port is open
     * @return true if a port is currently open
     */
    virtual bool IsPortOpen() const = 0;

    /**
     * Send a MIDI note on message
     * @param channel MIDI channel (0-15)
     * @param note Note number (0-127)
     * @param velocity Velocity (0-127)
     * @return true on success
     */
    virtual bool SendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) = 0;

    /**
     * Send a MIDI note off message
     * @param channel MIDI channel (0-15)
     * @param note Note number (0-127)
     * @param velocity Release velocity (0-127)
     * @return true on success
     */
    virtual bool SendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) = 0;

    /**
     * Send a MIDI control change message
     * @param channel MIDI channel (0-15)
     * @param controller Controller number (0-127)
     * @param value Controller value (0-127)
     * @return true on success
     */
    virtual bool SendControlChange(uint8_t channel, uint8_t controller, uint8_t value) = 0;

    /**
     * Send a MIDI program change message
     * @param channel MIDI channel (0-15)
     * @param program Program number (0-127)
     * @return true on success
     */
    virtual bool SendProgramChange(uint8_t channel, uint8_t program) = 0;

    /**
     * Send a raw MIDI message
     * @param data Message bytes
     * @param length Number of bytes
     * @return Number of bytes sent
     */
    virtual size_t SendRawMessage(const uint8_t* data, size_t length) = 0;

    /**
     * Poll for incoming MIDI messages
     * @param event Output event (if any)
     * @return true if a message was received
     */
    virtual bool PollMidiInput(MidiEvent& event) = 0;

    /**
     * Get last error message
     * @return Error description
     */
    virtual const char* GetErrorMessage() const = 0;

    /**
     * Check if backend is ready
     * @return true if initialized and ready to use
     */
    virtual bool IsReady() const = 0;
};

/**
 * @brief MIDI backend factory
 * 
 * Creates platform-appropriate MIDI backend instances
 */
class MidiBackendFactory {
public:
    /**
     * Create the default MIDI backend for current platform
     * On Windows: WindowsMidiBackend (MIDI API)
     * On Linux/macOS: RtMidiBackend
     * @return Pointer to new backend instance (caller owns)
     */
    static std::unique_ptr<IMidiBackend> Create();

    /**
     * Create Windows MIDI API backend
     * @return Pointer to new WindowsMidiBackend instance (caller owns)
     */
    static std::unique_ptr<IMidiBackend> CreateWindowsMidi();

    /**
     * Create RtMidi backend (cross-platform)
     * @return Pointer to new RtMidiBackend instance (caller owns)
     */
    static std::unique_ptr<IMidiBackend> CreateRtMidi();
};

#endif // MIDIBACKEND_H
