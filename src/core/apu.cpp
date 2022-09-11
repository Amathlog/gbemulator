#include <core/apu.h>
#include <core/bus.h>

using GBEmulator::APU;

APU::APU()
    : m_synth()
    , m_channel1(m_synth, 1)
    , m_channel2(m_synth, 2)
    , m_channel3(m_synth)
    , m_channel4(m_synth)
    , m_circularBuffer(1000000)
{
    m_synth.setOutputGen(((m_channel1.GetWave() + m_channel2.GetWave()) * 0.75f + m_channel3.GetWave() + m_channel4.GetWave()) / 4.0f);

    m_useTonic = false;
    m_currentTime = 0.0;
}

APU::~APU()
{
    Stop();
}

void APU::Stop()
{
    m_circularBuffer.Stop();
}

void APU::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    m_channel1.SerializeTo(visitor);
    m_channel2.SerializeTo(visitor);
    m_channel3.SerializeTo(visitor);
    m_channel4.SerializeTo(visitor);
    visitor.WriteValue(m_nbCycles);
}

void APU::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    m_channel1.DeserializeFrom(visitor);
    m_channel2.DeserializeFrom(visitor);
    m_channel3.DeserializeFrom(visitor);
    m_channel4.DeserializeFrom(visitor);
    visitor.ReadValue(m_nbCycles);
}

void APU::Reset()
{
    m_channel1.Reset();
    m_channel2.Reset();
    m_channel3.Reset();
    m_channel4.Reset();

    m_circularBuffer.Reset();
    m_bufferPtr = 0;
    m_currentTime = 0;
}

void APU::Clock()
{
    // APU is clock at 1.048576 MHz.
    // We clock the channels at 512 Hz, so every 2048 cycles
    if ((m_nbCycles & (size_t)0x07FF) == 0)
    {
        m_channel1.Update(m_synth);
        m_channel2.Update(m_synth);
        m_channel3.Update(m_synth);
        m_channel4.Update(m_synth);
    }

    constexpr double sampleTimePerCPUCycle = 4.0 / (GBEmulator::CPU_SINGLE_SPEED_FREQ_D);
    constexpr double sampleTimePerSystemSample = 1.0 / GBEmulator::APU_SAMPLE_RATE_D;

    if (!m_useTonic)
    {
        m_currentTime += sampleTimePerCPUCycle;
        if (m_currentTime >= sampleTimePerSystemSample)
        {
            m_currentTime -= sampleTimePerSystemSample;

            double sample = (m_channel1.GetSample() + m_channel2.GetSample() + m_channel3.GetSample() + m_channel4.GetSample()) / 4.0;
            m_internalBuffer[m_bufferPtr++] = (float)sample;
            m_internalBuffer[m_bufferPtr++] = (float)sample;
            
            if (m_bufferPtr == m_internalBuffer.max_size())
            {
                m_bufferPtr = 0;
                m_circularBuffer.WriteData(m_internalBuffer.data(), m_internalBuffer.max_size() * sizeof(float));
            }
        }
    }

    ++m_nbCycles;
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
    else if ((addr >= 0xFF1A && addr <= 0xFF1E) || (addr >= 0xFF30 && addr <= 0xFF3F))
    {
        m_channel3.WriteByte(addr, data);
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
        m_channel3.SetEnable(enable);
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
    else if ((addr >= 0xFF0A && addr <= 0xFF0E) || (addr >= 0xFF30 && addr <= 0xFF3F))
    {
        return m_channel3.ReadByte(addr);
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
        if (m_channel3.IsEnabled())
            res |= 0x04;
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

    if (m_useTonic)
    {
        m_synth.fillBufferOfFloats(outData, numFrames, numChannels);
    }
    else
    {
        m_circularBuffer.ReadData(outData, numFrames * numChannels * sizeof(float));
    }
}