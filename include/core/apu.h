#pragma once

#include <core/serializable.h>
#include <MyTonic.h>
#include <core/audio/pulseChannel.h>
#include <core/audio/noiseChannel.h>

namespace GBEmulator
{
    class APU : public ISerializable
    {
    public:
        APU();

        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        void Reset();
        void Clock();

        void WriteByte(uint16_t addr, uint8_t data);
        uint8_t ReadByte(uint16_t addr) const;

        Tonic::Synth* GetSynth() { return &m_synth; }
        void FillSamples(float *outData, unsigned int numFrames, unsigned int numChannels);

    private:
        Tonic::Synth m_synth;
        PulseChannel m_channel1;
        PulseChannel m_channel2;
        NoiseChannel m_channel4;

        size_t m_nbCycles = 0;
    };
}