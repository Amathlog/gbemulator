#pragma once

#include <core/serializable.h>
#include <MyTonic.h>
#include <core/audio/pulseChannel.h>
#include <core/audio/noiseChannel.h>
#include <core/audio/waveChannel.h>
#include <core/audio/circularBuffer.h>

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

        Tonic::Synth* GetSynth() { return &m_synth; }
        void FillSamples(float *outData, unsigned int numFrames, unsigned int numChannels);

    private:
        Tonic::Synth m_synth;
        PulseChannel m_channel1;
        PulseChannel m_channel2;
        WaveChannel m_channel3;
        NoiseChannel m_channel4;

        size_t m_nbCycles = 0;

        std::array<float, 128> m_internalBuffer;
        size_t m_bufferPtr = 0;
        CircularBuffer<float> m_circularBuffer;

        double currentTime = 0.0;
        bool m_useTonic;
    };
}