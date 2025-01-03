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
    // Will fill 128 samples (64 sampels left and right) if they are ready.
    bool FillSamplesIfReady(float* outData);

    uint8_t GetDivCounter() const { return m_divCounter; }

private:
    PulseChannel m_channel1;
    PulseChannel m_channel2;
    WaveChannel m_channel3;
    NoiseChannel m_channel4;

    VinRegister m_vinRegister;
    OutputTerminalRegister m_outputTerminalRegister;
    bool m_allSoundsOn = false;

    size_t m_nbCycles = 0;
    // Passed to all channels on update to know if they need to clock their length, enveloppe and/or sweep.
    uint8_t m_divCounter = 0;

    std::array<float, 128> m_internalBuffer;
    size_t m_bufferPtr = 0;
    CircularBuffer<float> m_circularBuffer;

    double m_currentTime = 0.0;
    bool m_samplesReady;
};
} // namespace GBEmulator