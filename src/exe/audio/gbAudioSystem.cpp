#include <MyTonic.h>
#include <exe/audio/gbAudioSystem.h>

using GBEmulatorExe::GBAudioSystem;

GBAudioSystem::GBAudioSystem(GBEmulator::Bus& bus, bool syncWithAudio, unsigned nbChannels, unsigned sampleRate, unsigned bufferFrames)
    : AudioSystem(nbChannels, sampleRate, bufferFrames)
    , m_bus(bus)
    , m_syncWithAudio(syncWithAudio)
{
    Tonic::setSampleRate((float)m_sampleRate);
}

int GBAudioSystem::RenderCallback(void *outputBuffer, void* /*inputBuffer*/, unsigned int nBufferFrames,
                    double /*streamTime*/, RtAudioStreamStatus /*status*/, void* /*userData*/)
{
    m_bus.GetAPU().FillSamples((float*)outputBuffer, nBufferFrames, m_nbChannels);
    return 0;
}