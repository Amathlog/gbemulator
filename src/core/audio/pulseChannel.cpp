#include <core/audio/pulseChannel.h>

using GBEmulator::PulseChannel;
using GBEmulator::PulseOscillator;

PulseOscillator::PulseOscillator(double sampleRate)
    : m_sampleRate(sampleRate)
{
    
}

double PulseOscillator::Tick()
{
    if (m_duty == 0.0 || m_freq == 0.0)
        return 0.0;

    m_phase += m_freq / m_sampleRate;

    while (m_phase > 1.0)
        m_phase -= 1.0f;

    while (m_phase < 0.0)
        m_phase += 1.0;

    return m_phase <= m_duty ? -1.0 : 1.0;
}

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
    , m_oscillator(Tonic::sampleRate())
{
    Tonic::ControlGenerator controlDutyPulse = synth.addParameter(GetDutyCycleParameterName(), 0.5f);
    Tonic::ControlGenerator controlFreqPulse = synth.addParameter(GetFrequencyParameterName(), 440.0f);
    Tonic::ControlGenerator controlOutputPulse = synth.addParameter(GetOutputParameterName());
    Tonic::ControlGenerator controlOutputEnvPulse = synth.addParameter(GetEnveloppeOutputParameterName());

    m_wave = controlOutputEnvPulse * controlOutputPulse * Tonic::RectWave().freq(controlFreqPulse).pwm(controlDutyPulse);
}

void PulseChannel::Update(Tonic::Synth& synth)
{
    // Should be called every 1/512 seconds
    bool clock256Hz = m_nbUpdateCalls % 2 == 0;
    bool clock128Hz = m_nbUpdateCalls % 4 == 2;
    bool clock64Hz = m_nbUpdateCalls % 8 == 7;

    // Update every n / 256 seconds
    if (m_enabled && m_lengthCounter != 0 && clock256Hz && --m_lengthCounter == 0)
    {
        if (m_freqMsbReg.lengthEnable)
        {
            SetEnable(false);
        }
        else
        {
            m_lengthCounter = m_waveReg.length;
        }
    }

    // Update every n / 128 seconds
    if (m_enabled && m_sweepReg.time != 0 && clock128Hz && --m_sweepCounter == 0)
    {
        Sweep();
    }

    // Enveloppe updated every n / 64 seconds
    if (m_enabled && m_volumeReg.nbEnveloppeSweep != 0 && clock64Hz && --m_volumeCounter == 0)
    {
        if (m_volumeReg.enveloppeDirection == 0 && m_volumeReg.initialVolume > 0)
        {
            // Decrease
            --m_volumeReg.initialVolume;
            m_volumeChanged = true;
        }
        else if (m_volumeReg.enveloppeDirection == 1 && m_volumeReg.initialVolume < 0x0F)
        {
            // Increase
            ++m_volumeReg.initialVolume;
            m_volumeChanged = true;
        }

        m_volumeCounter = m_volumeReg.nbEnveloppeSweep;
    }

    if (m_frequencyChanged)
    {
        synth.setParameter(GetFrequencyParameterName(), (float)m_frequency);
        m_oscillator.SetFrequency(m_frequency);
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
        m_oscillator.SetDuty(newDuty);
        m_dutyChanged = false;
    }

    if (m_enabledChanged)
    {
        synth.setParameter(GetOutputParameterName(), m_enabled ? 1.0f : 0.0f);
        m_enabledChanged = false;
    }

    if (m_volumeChanged)
    {
        synth.setParameter(GetEnveloppeOutputParameterName(), (float)m_volumeReg.initialVolume / 0x0F);
        m_volumeChanged = false;
    }

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
    m_sweepCounter = 0;
    m_volumeCounter = 0;
    m_enabled = false;
    m_frequency = 0.0;
    m_combinedFreq = 0x0000;

    m_frequencyChanged = false;
    m_dutyChanged = false;
    m_volumeChanged = false;
    m_enabledChanged = false;
    m_nbUpdateCalls = 0;
    
    m_oscillator.Reset();
}

void PulseChannel::WriteByte(uint16_t addr, uint8_t data)
{
    switch(addr)
    {
    case 0x00:
        // Sweep
        m_sweepReg.reg = data;
        Sweep();
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
        m_volumeCounter = m_volumeReg.nbEnveloppeSweep;
        m_volumeChanged = true;
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
        if (m_freqMsbReg.initial > 0)
            Restart();
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

double PulseChannel::GetSample()
{
    double currentVolume = (double)m_volumeReg.initialVolume / 0x0F;
    return m_enabled ? currentVolume * m_oscillator.Tick() : 0.0;
}

void PulseChannel::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteValue(m_sweepReg);
    visitor.WriteValue(m_waveReg);
    visitor.WriteValue(m_volumeReg);
    visitor.WriteValue(m_freqLsb);
    visitor.WriteValue(m_freqMsbReg);
    visitor.WriteValue(m_lengthCounter);
    visitor.WriteValue(m_sweepCounter);
    visitor.WriteValue(m_volumeCounter);
    visitor.WriteValue(m_enabled);

    visitor.WriteValue(m_frequency);
    visitor.WriteValue(m_combinedFreq);
    visitor.WriteValue(m_frequencyChanged);
    visitor.WriteValue(m_dutyChanged);
    visitor.WriteValue(m_enabledChanged);
    visitor.WriteValue(m_volumeChanged);
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
    visitor.ReadValue(m_sweepCounter);
    visitor.ReadValue(m_volumeCounter);
    visitor.ReadValue(m_enabled);

    visitor.ReadValue(m_frequency);
    visitor.ReadValue(m_combinedFreq);
    visitor.ReadValue(m_frequencyChanged);
    visitor.ReadValue(m_dutyChanged);
    visitor.ReadValue(m_enabledChanged);
    visitor.ReadValue(m_volumeChanged);
    visitor.ReadValue(m_nbUpdateCalls);
}

void PulseChannel::UpdateFreq()
{
    CheckOverflow(m_combinedFreq);

    if (m_combinedFreq == 0)
        return;

    m_frequency = 131072.0 / (2048.0 - m_combinedFreq);
    m_frequencyChanged = true;

    m_freqLsb = m_combinedFreq & 0x00FF;
    m_freqMsbReg.freqMsb = (m_combinedFreq & 0x0700) >> 8;
}

void PulseChannel::Restart()
{
    SetEnable(true);
    if (m_lengthCounter == 0)
        m_lengthCounter = 64;

    m_nbUpdateCalls = 0;
    m_volumeCounter = m_volumeReg.nbEnveloppeSweep;
    m_sweepCounter = m_sweepReg.time;
    m_volumeChanged = true;
    m_oscillator.Reset();
}

void PulseChannel::CheckOverflow(uint16_t newFreq)
{
    if (newFreq >= 2047)
    {
        SetEnable(false);
        m_combinedFreq = 0;
        m_sweepReg.time = 0;
    }
}

void PulseChannel::Sweep()
{
    if (m_sweepReg.time == 0)
        return;

    int16_t frqChange = m_combinedFreq >> m_sweepReg.shift;
    m_combinedFreq += m_sweepReg.decrease ? -frqChange : frqChange;
    m_sweepCounter = m_sweepReg.time;
    UpdateFreq();

    // Then re-do it once and check overflow only
    frqChange = m_combinedFreq >> m_sweepReg.shift;
    uint16_t newFreq = m_combinedFreq + (m_sweepReg.decrease ? -frqChange : frqChange);
    CheckOverflow(newFreq);
}