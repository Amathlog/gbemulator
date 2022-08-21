#include <core/audio/noiseChannel.h>

using GBEmulator::NoiseChannel;
using GBEmulator::NoiseOscillator;

NoiseOscillator::NoiseOscillator()
{
    m_sampleRate = Tonic::sampleRate();
}

void NoiseOscillator::Reset()
{
    m_nbSamplesPerRandomValue = 1;
    m_volume = 0.0;
    m_shiftRegister = 1;
    m_shiftRegLength = 15;
    m_currentSample = 0;
}

void NoiseOscillator::SetFrequency(double freq)
{
    if (freq == 0)
    {
        m_nbSamplesPerRandomValue = 1;
        m_volume = 0;
    }
    else
    {
        double signalPeriod = 1.0 / freq;
        unsigned newValue = (unsigned)(floor(signalPeriod * m_sampleRate));
        if (newValue != 0)
        {
            m_nbSamplesPerRandomValue = newValue;
        }
        else
        {
            m_nbSamplesPerRandomValue = 1;
            m_volume = 0.0f;
        }
    }
}

double NoiseOscillator::GetSample()
{
    if (m_volume == 0.0)
    {
        return 0.0;
    }

    double value = m_shiftRegister & 0x0001 ? m_volume : -m_volume;
    if (++m_currentSample == m_nbSamplesPerRandomValue)
    {
        m_currentSample = 0;
        uint16_t otherFeedback = (m_shiftRegister >> 1) & 0x0001;
        uint16_t feedback = (m_shiftRegister ^ otherFeedback) & 0x0001;
        m_shiftRegister = (feedback << (m_shiftRegLength - 1)) | (m_shiftRegister >> 1);
    }

    return value;
}

NoiseChannel::NoiseChannel(Tonic::Synth& synth)
{
}

void NoiseChannel::Update(Tonic::Synth& synth)
{
    if (!m_enabled)
        return;

    // Should be called every 1/512 seconds
    bool clock256Hz = m_nbUpdateCalls % 2 == 0;
    bool clock64Hz = m_nbUpdateCalls % 8 == 7;

    // Update every n / 256 seconds
    if (m_lengthCounter != 0 && clock256Hz && --m_lengthCounter == 0)
    {
        if (m_initialReg.lengthEnable)
        {
            m_enabled = false;
            m_noise.setVolume(0);
            m_oscillator.m_volume = 0.0;
        }
        else
        {
            m_lengthCounter = m_lengthReg.length;
        }
    }

    // Enveloppe updated every n / 64 seconds
    if (m_volumeReg.nbEnveloppeSweep != 0 && clock64Hz && --m_volumeCounter == 0)
    {
        if (m_volumeReg.enveloppeDirection == 0 && m_volumeReg.initialVolume > 0)
        {
            // Decrease
            --m_volumeReg.initialVolume;
        }
        else if (m_volumeReg.enveloppeDirection == 1 && m_volumeReg.initialVolume < 0x0F)
        {
            // Increase
            ++m_volumeReg.initialVolume;
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
    m_lengthCounter = 0;
    m_volumeCounter = 0x00;
    m_enabled = false;

    m_nbUpdateCalls = 0;
    m_noise.reset();
    m_oscillator.Reset();
}

void NoiseChannel::SetVolume()
{
    float newVolume = (float)m_volumeReg.initialVolume / 0x0F;
    m_noise.setVolume(newVolume);
    m_oscillator.m_volume = newVolume;
}

void NoiseChannel::WriteByte(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
    case 0xFF20:
        // Length
        m_lengthReg.reg = data;
        m_lengthCounter = m_lengthReg.length;
        break;
    case 0xFF21:
        // Enveloppe
        m_volumeReg.reg = data;
        m_volumeCounter = m_volumeReg.nbEnveloppeSweep;
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
        if (m_initialReg.initial > 0 && !m_enabled)
        {
            m_enabled = true;
            if (m_lengthCounter == 0)
                m_lengthCounter = 64;
            m_volumeCounter = m_volumeReg.nbEnveloppeSweep;
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
    case 0xFF20:
        // Length
        return m_lengthReg.reg;
    case 0xFF21:
        // Enveloppe
        return m_volumeReg.reg;
    case 0xFF22:
        // Freq
        return m_polyReg.reg;
    case 0xFF23:
        // Initial
        return m_initialReg.reg;
    default:
        break;
    }

    return 0x00;
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
}

void NoiseChannel::SetFrequency()
{
    unsigned freq = 524288;
    float ratio = m_polyReg.ratio == 0 ? 0.5f : float(m_polyReg.ratio);

    freq >>= (m_polyReg.freq + 1);
    float newFreq = freq / ratio;
    m_noise.setFreq(newFreq);
    m_oscillator.SetFrequency(newFreq);
}