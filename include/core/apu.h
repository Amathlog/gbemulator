#pragma once

#include <core/audio/circularBuffer.h>
#include <core/audio/noiseChannel.h>
#include <core/audio/pulseChannel.h>
#include <core/audio/waveChannel.h>
#include <core/serializable.h>

namespace GBEmulator
{
class APU : public ISerializable
{
public:
    APU();
    ~APU();

    void SerializeTo(Utils::IWriteVisitor& visitor) const override;
    void DeserializeFrom(Utils::IReadVisitor& visitor) override;

    void Reset();
    void Clock();

    void Stop();

    void WriteByte(uint16_t addr, uint8_t data);
    uint8_t ReadByte(uint16_t addr) const;

    void FillSamples(float* outData, unsigned int numFrames, unsigned int numChannels);

private:
    PulseChannel m_channel1;
    PulseChannel m_channel2;
    WaveChannel m_channel3;
    NoiseChannel m_channel4;

    VinRegister m_vinRegister;
    OutputTerminalRegister m_outputTerminalRegister;
    bool m_allSoundsOn = false;

    size_t m_nbCycles = 0;

    std::array<float, 128> m_internalBuffer;
    size_t m_bufferPtr = 0;
    CircularBuffer<float> m_circularBuffer;

    double m_currentTime = 0.0;
};
} // namespace GBEmulator