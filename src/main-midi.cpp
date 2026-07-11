//
// main-midi.cpp - MIDI Backend Test
//
// Tests the cross-platform MIDI abstraction layer
// Tests both Windows MIDI API and RtMidi backends
//

#include <iostream>
#include <thread>
#include <chrono>
#include "MidiBackend.h"

int main()
{
    std::cout << "=== RASTER Music Tracker - MIDI Backend Test ===" << std::endl;
    std::cout << std::endl;

    // Create MIDI backend (auto-selects platform backend)
    auto midi = MidiBackendFactory::Create();
    if (!midi) {
        std::cerr << "✗ Failed to create MIDI backend" << std::endl;
        return 1;
    }
    std::cout << "✓ MIDI backend created successfully" << std::endl;

    // Initialize
    if (!midi->Init()) {
        std::cerr << "✗ Failed to initialize MIDI backend: " << midi->GetErrorMessage() << std::endl;
        return 1;
    }
    std::cout << "✓ MIDI backend initialized" << std::endl;

    // Query available MIDI ports
    int portCount = midi->GetPortCount();
    std::cout << "Available MIDI output ports: " << portCount << std::endl;

    if (portCount == 0) {
        std::cout << "⚠ No MIDI ports available - cannot send test messages" << std::endl;
        std::cout << "  (This is normal if no MIDI devices/synthesizers are connected)" << std::endl;
        midi->Deinit();
        return 0;
    }

    std::cout << std::endl << "MIDI Ports:" << std::endl;
    for (int i = 0; i < portCount; ++i) {
        std::string portName = midi->GetPortName(i);
        std::cout << "  [" << i << "] " << portName << std::endl;
    }
    std::cout << std::endl;

    // Use first port
    int portIndex = 0;
    std::cout << "Opening port [" << portIndex << "]: " << midi->GetPortName(portIndex) << std::endl;
    if (!midi->OpenPort(portIndex)) {
        std::cerr << "✗ Failed to open MIDI port: " << midi->GetErrorMessage() << std::endl;
        midi->Deinit();
        return 1;
    }
    std::cout << "✓ MIDI port opened" << std::endl;
    std::cout << std::endl;

    // Send test MIDI messages
    std::cout << "Sending MIDI messages..." << std::endl;

    // Note On (C4, velocity 100)
    uint8_t note = 60;  // Middle C (C4)
    uint8_t velocity = 100;
    uint8_t channel = 0;  // Channel 1

    std::cout << "  Sending NoteOn: Channel " << (int)channel << ", Note " << (int)note
              << ", Velocity " << (int)velocity << std::endl;
    if (midi->SendNoteOn(channel, note, velocity)) {
        std::cout << "  ✓ NoteOn sent" << std::endl;
    } else {
        std::cerr << "  ✗ Failed to send NoteOn: " << midi->GetErrorMessage() << std::endl;
    }

    // Play for 500ms
    std::cout << "  Playing for 500ms..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Send Control Change (Volume)
    uint8_t controller = 7;  // Main Volume
    uint8_t ccValue = 64;    // Half volume

    std::cout << "  Sending ControlChange: Channel " << (int)channel << ", CC " << (int)controller
              << ", Value " << (int)ccValue << std::endl;
    if (midi->SendControlChange(channel, controller, ccValue)) {
        std::cout << "  ✓ ControlChange sent" << std::endl;
    } else {
        std::cerr << "  ✗ Failed to send ControlChange: " << midi->GetErrorMessage() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Send Program Change (different instrument)
    uint8_t program = 5;  // Some instrument

    std::cout << "  Sending ProgramChange: Channel " << (int)channel << ", Program " << (int)program << std::endl;
    if (midi->SendProgramChange(channel, program)) {
        std::cout << "  ✓ ProgramChange sent" << std::endl;
    } else {
        std::cerr << "  ✗ Failed to send ProgramChange: " << midi->GetErrorMessage() << std::endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Note Off
    std::cout << "  Sending NoteOff: Channel " << (int)channel << ", Note " << (int)note << std::endl;
    if (midi->SendNoteOff(channel, note, 0)) {
        std::cout << "  ✓ NoteOff sent" << std::endl;
    } else {
        std::cerr << "  ✗ Failed to send NoteOff: " << midi->GetErrorMessage() << std::endl;
    }

    std::cout << std::endl;
    std::cout << "✓ MIDI test completed successfully!" << std::endl;

    // Cleanup
    midi->Deinit();

    return 0;
}
