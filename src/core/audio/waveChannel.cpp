#include <core/audio/waveChannel.h>

using GBEmulator::WaveChannel;

WaveChannel::WaveChannel(Tonic::Synth& synth)
{
}

void WaveChannel::Update(Tonic::Synth& synth)
{
    if (!m_enabled)
        return;

    // Should be called every 1/512 seconds
    bool clock256Hz = m_nbUpdateCalls % 2 == 0;

    // Update every n / 256 seconds
    if (m_lengthCounter != 0 && clock256Hz && --m_lengthCounter == 0)
    {
        if (m_freqMsbReg.lengthEnable)
        {
            m_enabled = false;
            m_wave.setVolume(0);
        }
        else
        {
            m_lengthCounter = m_soundLength;
        }
    }

    m_nbUpdateCalls++;
}

void WaveChannel::Reset()
{
    m_soundLength = 0x00;
    m_volumeReg.reg = 0x00;
    m_freqMsbReg.reg = 0x00;
    m_lengthCounter = 0;
    m_enabled = false;
    m_freq = 0x0000;
    m_wave.reset();

    m_nbUpdateCalls = 0;
}

void WaveChannel::WriteByte(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
    case 0xFF1A:
        // Enable
        m_enabled = (data & 0x80) > 0;
        SetVolume();

        break;
    case 0xFF1B:
        // Sound length
        m_soundLength = data;
        m_lengthCounter = m_soundLength;
        break;
    case 0xFF1C:
    {
        // Volume
        m_volumeReg.reg = data;
        SetVolume();
        break;
    }
    case 0xFF1D:
        // Freq LSB
        m_freq = (m_freq & 0x0700) | data;
        SetFrequency();
        break;
    case 0xFF1E:
        // Freq MSB
        m_freqMsbReg.reg = data;
        m_freq = ((uint16_t)m_freqMsbReg.freqMsb << 8) | (m_freq & 0x00FF);
        if (m_freqMsbReg.initial > 0)
            Restart();

        break;
    default:
        break;
    }

    if (addr >= 0xFF30 && addr <= 0xFF3F)
    {
        // Wave Table. Only accessible if channel disabled
        uint8_t position = (addr - 0xFF30) * 2;
        uint8_t firstSample = (data & 0xF0) >> 4;
        uint8_t secondSample = data & 0x0F;
        m_wave.setSample(firstSample, position);
        m_wave.setSample(secondSample, position + 1);
    }
}

uint8_t WaveChannel::ReadByte(uint16_t addr) const
{
    switch (addr)
    {
    case 0xFF1A:
        return m_enabled ? 0x80 : 0x00;
    case 0xFF1B:
        return m_soundLength;
    case 0xFF1C:
        return m_volumeReg.reg;
    case 0xFF1D:
        return (uint8_t)(m_freq & 0x00FF);
    case 0xFF1E:
        return m_freqMsbReg.reg;
    }

    if (addr >= 0xFF30 && addr <= 0xFF3F && !m_enabled)
    {
        // Wave Table. Only accessible if channel disabled
        uint8_t position = (addr - 0xFF30) * 2;
        uint8_t firstSample = m_wave.getSample(position);
        uint8_t secondSample = m_wave.getSample(position + 1);
        return ((firstSample & 0x0F) << 4) | (secondSample & 0x0F);
    }

    return 0x00;
}

void WaveChannel::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteValue(m_volumeReg);
    visitor.WriteValue(m_soundLength);
    visitor.WriteValue(m_lengthCounter);
    visitor.WriteValue(m_freqMsbReg);
    visitor.WriteValue(m_freq);
    visitor.WriteValue(m_enabled);
    visitor.WriteValue(m_nbUpdateCalls);
}

void WaveChannel::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    visitor.ReadValue(m_volumeReg);
    visitor.ReadValue(m_soundLength);
    visitor.ReadValue(m_lengthCounter);
    visitor.ReadValue(m_freqMsbReg);
    visitor.ReadValue(m_freq);
    visitor.ReadValue(m_enabled);
    visitor.ReadValue(m_nbUpdateCalls);
}

void WaveChannel::SetFrequency()
{
    m_wave.setFreq(65536.0f / (2048 - m_freq));
}

void WaveChannel::SetVolume()
{        
    float newVolume = 0.f;
    if (m_enabled)
    {
        switch (m_volumeReg.volume)
        {
        case 1:
            newVolume = 1.f;
            break;
        case 2:
            newVolume = 0.5f;
            break;
        case 3:
            newVolume = 0.25f;
            break;
        default:
            newVolume = 0.0f;
            break;
        }
    }

    m_wave.setVolume(newVolume);
}

void WaveChannel::Restart()
{
    //m_enabled = true;
    m_lengthCounter = 0xFF;
    SetFrequency();
    SetVolume();
    m_wave.resetPosition();
}