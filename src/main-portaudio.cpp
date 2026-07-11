// Test main for PortAudio backend (Phase 2)
// This is only for core-only builds

#include "AudioBackend.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <cmath>

int main()
{
    std::cout << "=== RASTER Music Tracker - PortAudio Audio Backend Test ===" << std::endl;
    
    // Create a PortAudio backend
    auto backend = AudioBackendFactory::CreatePortAudio();
    if (!backend)
    {
        std::cerr << "Failed to create PortAudio backend!" << std::endl;
        return 1;
    }
    
    std::cout << "✓ PortAudio backend created successfully" << std::endl;
    
    // Setup audio format (44100 Hz, 16-bit mono)
    AudioFormat fmt(1, 44100, 16);
    
    std::cout << "Initializing audio with format:" << std::endl;
    std::cout << "  Channels: " << fmt.channels << std::endl;
    std::cout << "  Sample Rate: " << fmt.sample_rate << " Hz" << std::endl;
    std::cout << "  Bits per Sample: " << fmt.bits_per_sample << std::endl;
    
    if (!backend->Init(fmt))
    {
        std::cerr << "Failed to initialize audio backend!" << std::endl;
        std::cerr << "Error: " << backend->GetErrorMessage() << std::endl;
        return 1;
    }
    
    std::cout << "✓ Audio backend initialized" << std::endl;
    std::cout << "Buffer Size: " << backend->GetBufferSize() << " bytes" << std::endl;
    std::cout << "Available Space: " << backend->GetAvailableSpace() << " bytes" << std::endl;
    
    // Try to start audio
    if (!backend->Start())
    {
        std::cerr << "Failed to start audio playback!" << std::endl;
        std::cerr << "Error: " << backend->GetErrorMessage() << std::endl;
        backend->Deinit();
        return 1;
    }
    
    std::cout << "✓ Audio playback started" << std::endl;
    std::cout << "Is Playing: " << (backend->IsPlaying() ? "Yes" : "No") << std::endl;
    
    // Write a simple test signal (sine wave)
    const size_t SAMPLE_COUNT = fmt.sample_rate / 10; // 100ms of audio
    int16_t* buffer = new int16_t[SAMPLE_COUNT];
    
    // Generate a 440 Hz sine wave
    for (size_t i = 0; i < SAMPLE_COUNT; i++)
    {
        const double frequency = 440.0;
        const double amplitude = 32000.0; // Near max int16
        const double phase = 2.0 * 3.141592653589793 * frequency * i / fmt.sample_rate;
        buffer[i] = (int16_t)(amplitude * sin(phase));
    }
    
    std::cout << "\nWriting test signal (440 Hz sine, 100ms)..." << std::endl;
    size_t written = backend->Write(buffer, SAMPLE_COUNT * sizeof(int16_t));
    std::cout << "Bytes written: " << written << " / " << (SAMPLE_COUNT * sizeof(int16_t)) << std::endl;
    
    // Let it play
    std::cout << "Playback duration: 200ms..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "\n✓ Audio test completed successfully!" << std::endl;
    
    // Clean up
    backend->Stop();
    backend->Deinit();
    delete[] buffer;
    
    return 0;
}
