#include <core/audio/pulseChannel.h>

using GBEmulator::PulseChannel;

std::string PulseChannel::GetDutyCycleParameterName()
{
    return std::string("dutyCyclePulse") + std::to_string(m_number);
}

std::string PulseChannel::GetFrequencyParameterName()
{
    return std::string("freqPulse") + std::to_string(m_number);
}

std::string PulseChannel::GetOutputParameterName()
{
    return std::string("outputPulse") + std::to_string(m_number);
}

std::string PulseChannel::GetEnveloppeOutputParameterName()
{
    return std::string("outputEnvPulse") + std::to_string(m_number);
}

PulseChannel::PulseChannel(Tonic::Synth& synth, int number)
    : m_number(number)
{
    Tonic::ControlGenerator controlDutyPulse = synth.addParameter(GetDutyCycleParameterName(), 0.5f);
    Tonic::ControlGenerator controlFreqPulse = synth.addParameter(GetFrequencyParameterName(), 440.0f);
    Tonic::ControlGenerator controlOutputPulse = synth.addParameter(GetOutputParameterName());
    Tonic::ControlGenerator controlOutputEnvPulse = synth.addParameter(GetEnveloppeOutputParameterName());

    m_wave = controlOutputEnvPulse * controlOutputPulse * Tonic::RectWave().freq(controlFreqPulse).pwm(controlDutyPulse);
}

void PulseChannel::Update(Tonic::Synth& synth)
{
    // Should be called every 1/256 seconds
    if (m_lengthCounter != 0 && --m_lengthCounter == 0)
    {
        if (m_freqMsbReg.counterConsecutiveSelection)
        {
            m_enabled = false;
        }
        else
        {
            m_lengthCounter = m_waveReg.length;
        }
    }

    if (m_sweepReg.time != 0 && m_nbUpdateCalls % ((uint8_t)m_sweepReg.time * 2) == 0)
    {
        uint16_t frqChange = m_combinedFreq >> m_sweepReg.shift;
        if (m_sweepReg.decrease)
        {
            m_combinedFreq = m_combinedFreq == 0 ? m_combinedFreq : m_combinedFreq - frqChange;
        }
        else
        {
            m_combinedFreq = m_combinedFreq == 0x07FF ? m_combinedFreq : m_combinedFreq + frqChange;
        }
        UpdateFreq();
    }

    // Enveloppe updated every n / 64 seconds
    if (m_volumeReg.nbEnveloppeSweep != 0 && (m_nbUpdateCalls % ((uint8_t) m_volumeReg.nbEnveloppeSweep * 4)) == 0)
    {
        if (m_volumeReg.enveloppeDirection == 0)
        {
            // Decrease
            m_volumeReg.initialVolume = m_volumeReg.initialVolume == 0 ? 0 : (m_volumeReg.initialVolume - 1);
        }
        else
        {
            // Increase
            m_volumeReg.initialVolume = m_volumeReg.initialVolume == 0x0F ? 0x0F : (m_volumeReg.initialVolume + 1);
        }
    }

    if (m_frequencyChanged)
    {
        synth.setParameter(GetFrequencyParameterName(), (float)m_frequency);
        m_frequencyChanged = false;
    }

    if (m_dutyChanged)
    {
        float newDuty = 0.125f;
        if (m_waveReg.duty == 0x01)
        {
            newDuty = 0.25f;
        }
        else if (m_waveReg.duty == 0x02)
        {
            newDuty = 0.5f;
        }
        else if (m_waveReg.duty == 0x03)
        {
            newDuty = 0.75f;
        }

        synth.setParameter(GetDutyCycleParameterName(), newDuty);
    }

    synth.setParameter(GetOutputParameterName(), m_enabled ? 1.0f : 0.0f);
    synth.setParameter(GetEnveloppeOutputParameterName(), (float)m_volumeReg.initialVolume / 0x0F);
    m_nbUpdateCalls++;
}

void PulseChannel::Reset()
{
    m_sweepReg.reg = 0x00;
    m_waveReg.reg = 0x00;
    m_volumeReg.reg = 0x00;
    m_freqLsb = 0x00;
    m_freqMsbReg.reg = 0x00;
    m_lengthCounter = 0;
    m_enabled = false;
    m_frequency = 0.0;
    m_combinedFreq = 0x0000;

    m_frequencyChanged = false;
    m_dutyChanged = false;
    m_nbUpdateCalls = 0;
}

void PulseChannel::WriteByte(uint16_t addr, uint8_t data)
{
    switch(addr)
    {
    case 0x00:
        // Sweep
        m_sweepReg.reg = data;
        break;
    case 0x01:
        // Wave
        m_waveReg.reg = data;
        m_lengthCounter = m_waveReg.length;
        m_dutyChanged = true;
        break;
    case 0x02:
        // Enveloppe
        m_volumeReg.reg = data;
        break;
    case 0x3:
        // Freq lsb
        m_freqLsb = data;
        m_combinedFreq = ((uint16_t)m_freqMsbReg.freqMsb << 8) | m_freqLsb;
        UpdateFreq();
        break;
    case 0x04:
        // Freq Msb
        m_freqMsbReg.reg = data;
        m_combinedFreq = ((uint16_t)m_freqMsbReg.freqMsb << 8) | m_freqLsb;
        m_enabled = m_freqMsbReg.initial > 0;
        UpdateFreq();
        break;
    default:
        break;
    }
}

uint8_t PulseChannel::ReadByte(uint16_t addr) const
{
    switch(addr)
    {
    case 0x00:
        // Sweep
        return m_sweepReg.reg;
    case 0x01:
        // Wave
        return m_waveReg.reg;
    case 0x02:
        // Enveloppe
        return m_volumeReg.reg;
    case 0x03:
        // Freq lsb
        return (uint8_t)(m_combinedFreq & 0x00FF);
    case 0x04:
    {
        // Freq Msb
        FrequencyHighRegister regCopy = m_freqMsbReg;
        regCopy.freqMsb = m_combinedFreq >> 8;
        return m_freqMsbReg.reg;
    }
    default:
        break;
    }

    return 0x00;
}

void PulseChannel::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteValue(m_sweepReg);
    visitor.WriteValue(m_waveReg);
    visitor.WriteValue(m_volumeReg);
    visitor.WriteValue(m_freqLsb);
    visitor.WriteValue(m_freqMsbReg);
    visitor.WriteValue(m_lengthCounter);
    visitor.WriteValue(m_enabled);

    visitor.WriteValue(m_frequency);
    visitor.WriteValue(m_combinedFreq);
    visitor.WriteValue(m_frequencyChanged);
    visitor.WriteValue(m_dutyChanged);
    visitor.WriteValue(m_nbUpdateCalls);
}

void PulseChannel::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    visitor.ReadValue(m_sweepReg);
    visitor.ReadValue(m_waveReg);
    visitor.ReadValue(m_volumeReg);
    visitor.ReadValue(m_freqLsb);
    visitor.ReadValue(m_freqMsbReg);
    visitor.ReadValue(m_lengthCounter);
    visitor.ReadValue(m_enabled);

    visitor.ReadValue(m_frequency);
    visitor.ReadValue(m_combinedFreq);
    visitor.ReadValue(m_frequencyChanged);
    visitor.ReadValue(m_dutyChanged);
    visitor.ReadValue(m_nbUpdateCalls);
}

void PulseChannel::UpdateFreq()
{
    m_frequency = m_combinedFreq < 2038 ? 131072.0 / (2048.0 - m_combinedFreq) : 0.0;
    m_frequencyChanged = true;
}