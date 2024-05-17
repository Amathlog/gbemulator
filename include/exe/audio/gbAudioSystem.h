#pragma once

#include <core/bus.h>
#include <exe/audio/audioSystem.h>

namespace GBEmulatorExe
{
class GBAudioSystem : public AudioSystem
{
public:
    GBAudioSystem(GBEmulator::Bus& bus, bool syncWithAudio, unsigned nbChannels = 2, unsigned sampleRate = 44100,
                  unsigned bufferFrames = 256);
    ~GBAudioSystem() = default;

    int RenderCallback(void* outputBuffer, void* inputBuffer, unsigned int nBufferFrames, double streamTime,
                       RtAudioStreamStatus status, void* userData) override;

private:
    GBEmulator::Bus& m_bus;
    bool m_syncWithAudio;
};
} // namespace GBEmulatorExe