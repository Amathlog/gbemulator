#include <core/audio/noiseChannel.h>
#include <core/constants.h>

using GBEmulator::NoiseChannel;
using GBEmulator::NoiseOscillator;

NoiseOscillator::NoiseOscillator() { m_realSampleDuration = 1.0 / GBEmulator::APU_SAMPLE_RATE_D; }

void NoiseOscillator::Reset()
{
    m_shiftRegister = 1;
    m_shiftRegLength = 15;
    m_sampleDuration = 0.0;
    m_volume = 0.0;
    m_elaspedTime = 0.0;
}

void NoiseOscillator::SetFrequency(double freq)
{
    if (freq == 0.0)
    {
        m_sampleDuration = 0.0;
        m_volume = 0.0;
    }
    else
    {
        m_sampleDuration = 1.0 / freq;
    }
}

double NoiseOscillator::GetSample()
{
    if (m_volume == 0.0)
    {
        return 0.0;
    }

    double value = m_shiftRegister & 0x0001 ? m_volume : -m_volume;
    m_elaspedTime += m_realSampleDuration;
    if (m_elaspedTime >= m_sampleDuration)
    {
        m_elaspedTime -= m_sampleDuration;
        uint16_t otherFeedback = (m_shiftRegister >> 1) & 0x0001;
        uint16_t feedback = (m_shiftRegister ^ otherFeedback) & 0x0001;
        m_shiftRegister = (feedback << (m_shiftRegLength - 1)) | (m_shiftRegister >> 1);
    }

    return value;
}

void NoiseChannel::Update()
{
    if (!m_enabled)
        return;

    // Should be called every 1/512 seconds
    bool clock256Hz = m_nbUpdateCalls % 2 == 0;
    bool clock64Hz = m_nbUpdateCalls % 8 == 7;

    // Update every n / 256 seconds
    if (m_initialReg.lengthEnable && m_lengthCounter != 0 && clock256Hz && --m_lengthCounter == 0)
    {
        if (m_enabled)
        {
            m_enabled = false;
            m_oscillator.m_volume = 0.0;
            m_volume = 0;
        }
    }

    // Enveloppe updated every n / 64 seconds
    if (m_volumeReg.nbEnveloppeSweep != 0 && clock64Hz && --m_volumeCounter == 0)
    {
        if (m_volumeReg.enveloppeDirection == 0 && m_volume > 0)
        {
            // Decrease
            --m_volume;
        }
        else if (m_volumeReg.enveloppeDirection == 1 && m_volume < 0x0F)
        {
            // Increase
            ++m_volume;
        }

        SetVolume();
        m_volumeCounter = m_volumeReg.nbEnveloppeSweep;
    }

    m_nbUpdateCalls++;
}

void NoiseChannel::Reset()
{
    m_lengthReg.reg = 0x00;
    m_volumeReg.reg = 0x00;
    m_initialReg.reg = 0x00;
    m_polyReg.reg = 0x00;
    m_lengthCounter = 0;
    m_volumeCounter = 0x00;
    m_enabled = false;
    m_volume = 0;

    m_nbUpdateCalls = 0;
    m_oscillator.Reset();
}

void NoiseChannel::SetVolume() { m_oscillator.m_volume = (float)m_volume / 0x0F; }

void NoiseChannel::WriteByte(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
    case 0xFF1F:
        // Not used
        break;
    case 0xFF20:
        // Length
        m_lengthReg.reg = data;
        m_lengthCounter = 64 - m_lengthReg.length;
        break;
    case 0xFF21:
        // Enveloppe
        m_volumeReg.reg = data;
        m_volumeCounter = m_volumeReg.nbEnveloppeSweep;
        m_volume = m_volumeReg.initialVolume;
        SetEnable(m_volumeReg.initialVolume > 0);
        SetVolume();
        break;
    case 0xFF22:
        // Freq
        m_polyReg.reg = data;
        SetFrequency();
        break;
    case 0xFF23:
        // Initial
        m_initialReg.reg = data;
        if (m_initialReg.initial > 0)
        {
            m_enabled = true;
            if (m_lengthCounter == 0)
                m_lengthCounter = 64;
            m_volumeCounter = m_volumeReg.nbEnveloppeSweep;
            m_volume = m_volumeReg.initialVolume;
            SetVolume();
        }
    default:
        break;
    }
}

uint8_t NoiseChannel::ReadByte(uint16_t addr) const
{
    switch (addr)
    {
    case 0xFF1F:
        return 0xFF;
    case 0xFF20:
        // Length
        return 0xFF;
    case 0xFF21:
        // Enveloppe
        return m_volumeReg.reg;
    case 0xFF22:
        // Freq
        return m_polyReg.reg;
    case 0xFF23:
        // Initial
        return m_initialReg.reg | 0xBF;
    default:
        break;
    }

    return 0xFF;
}

void NoiseChannel::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteValue(m_lengthReg);
    visitor.WriteValue(m_volumeReg);
    visitor.WriteValue(m_polyReg);
    visitor.WriteValue(m_initialReg);
    visitor.WriteValue(m_lengthCounter);
    visitor.WriteValue(m_volumeCounter);
    visitor.WriteValue(m_enabled);
    visitor.WriteValue(m_nbUpdateCalls);
    visitor.WriteValue(m_volume);
}

void NoiseChannel::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    visitor.ReadValue(m_lengthReg);
    visitor.ReadValue(m_volumeReg);
    visitor.ReadValue(m_polyReg);
    visitor.ReadValue(m_initialReg);
    visitor.ReadValue(m_lengthCounter);
    visitor.ReadValue(m_volumeCounter);
    visitor.ReadValue(m_enabled);
    visitor.ReadValue(m_nbUpdateCalls);
    visitor.ReadValue(visitor);
}

void NoiseChannel::SetFrequency()
{
    unsigned freq = 524288;
    float ratio = m_polyReg.ratio == 0 ? 0.5f : float(m_polyReg.ratio);

    freq >>= (m_polyReg.freq + 1);
    float newFreq = freq / ratio;
    m_oscillator.SetFrequency(newFreq);

    m_oscillator.m_shiftRegLength = m_polyReg.width == 1 ? 7 : 15;
}