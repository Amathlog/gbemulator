#include <core/apu.h>
#include <core/bus.h>

using GBEmulator::APU;

APU::APU()
    : m_synth()
    , m_channel1(m_synth, 1)
    , m_channel2(m_synth, 2)
    , m_channel4(m_synth)
{
    m_synth.setOutputGen((m_channel1.GetWave() + m_channel2.GetWave() + m_channel4.GetWave()) / 3.0f);
}

void APU::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    m_channel1.SerializeTo(visitor);
    m_channel2.SerializeTo(visitor);
    m_channel4.SerializeTo(visitor);
    visitor.WriteValue(m_nbCycles);
}

void APU::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    m_channel1.DeserializeFrom(visitor);
    m_channel2.DeserializeFrom(visitor);
    m_channel4.DeserializeFrom(visitor);
    visitor.ReadValue(m_nbCycles);
}

void APU::Reset()
{
    m_channel1.Reset();
    m_channel2.Reset();
    m_channel4.Reset();
}

void APU::Clock()
{
    // APU is clock at 2.097152 MHz.
    // We clock the channels at 512 Hz, so every 4096 cycles
    if (m_nbCycles++ % 4096 == 0)
    {
        m_channel1.Update(m_synth);
        m_channel2.Update(m_synth);
        m_channel4.Update(m_synth);
    }
}

void APU::WriteByte(uint16_t addr, uint8_t data)
{
    if (addr >= 0xFF10 && addr <= 0xFF14)
    {
        m_channel1.WriteByte(addr - 0xFF10, data);
    }
    else if (addr >= 0xFF16 && addr <= 0xFF19)
    {
        m_channel2.WriteByte(addr - 0xFF15, data);
    }
    else if (addr >= 0xFF20 && addr <= 0xFF23)
    {
        m_channel4.WriteByte(addr, data);
    }
    else if (addr == 0xFF26)
    {
        bool enable = (data & 0x80) > 0;
        m_channel1.SetEnable(enable);
        m_channel2.SetEnable(enable);
        m_channel4.SetEnable(enable);
    }
}

uint8_t APU::ReadByte(uint16_t addr) const
{
    if (addr >= 0xFF10 && addr <= 0xFF14)
    {
        return m_channel1.ReadByte(addr - 0xFF10);
    }
    else if (addr >= 0xFF16 && addr <= 0xFF19)
    {
        return m_channel2.ReadByte(addr - 0xFF15);
    }
    else if (addr >= 0xFF20 && addr <= 0xFF23)
    {
        return m_channel4.ReadByte(addr);
    }
    else if (addr == 0xFF26)
    {
        uint8_t res = 0x00;
        if (m_channel1.IsEnabled())
            res |= 0x01;
        if (m_channel2.IsEnabled())
            res |= 0x02;
        if (m_channel4.IsEnabled())
            res |= 0x08;

        return res;
    }
    else if (addr >= 0xFF27 && addr <= 0xFF2F)
    {
        return 0xFF;
    }

    return 0x00;
}

void APU::FillSamples(float *outData, unsigned int numFrames, unsigned int numChannels)
{
    //std::unique_lock<std::mutex> lk(m_lock);
    m_synth.fillBufferOfFloats(outData, numFrames, numChannels);
}