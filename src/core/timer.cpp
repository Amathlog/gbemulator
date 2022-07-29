#include <core/timer.h>
#include <core/constants.h>

using GBEmulator::Timer;

Timer::Timer()
{
    Reset();
}

uint8_t Timer::ReadByte(uint16_t addr, bool /*readOnly*/)
{
    uint8_t data = 0x00;

    switch (addr)
    {
    case 0xFF04:
        data = m_divider;
        break;
    case 0xFF05:
        data = m_timerCounter;
        break;
    case 0xFF06:
        data = m_timerModulo;
        break;
    case 0xFF07:
        data = m_timerControl;
        break;
    default:
        break;
    }

    return data;
}

void Timer::WriteByte(uint16_t addr, uint8_t data)
{
    switch (addr)
    {
    case 0xFF04:
        m_divider = 0x00; // Any write to the divider register resets it.
        break;
    case 0xFF05:
        m_timerCounter = data;
        break;
    case 0xFF06:
        m_timerModulo = data;
        break;
    case 0xFF07:
        m_timerControl = data;
        m_enabled = !!(m_timerControl & 0x04);

        switch(m_timerControl & 0x03)
        {
        case 0:
            m_timerControlValue = 4096; // CPU Clock / 1024
            break;
        case 1:
            m_timerControlValue = 262144; // CPU Clock / 16
            break;
        case 2:
            m_timerControlValue = 65536; // CPU Clock / 64
            break;
        case 3:
        default:
            m_timerControlValue = 16384; // CPU Clock / 256
            break;
        }

        break;
    default:
        break;
    }
}

bool Timer::Clock()
{
    m_nbClocks = (m_nbClocks + 1) % GBEmulator::CPU_SINGLE_SPEED_FREQ;

    // Divider is always incremented at a speed of 16384Hz (double in double speed mode)
    if (m_nbClocks % 16384 == 0)
        m_divider++;

    // If the timer is not enabled exit there
    if (!m_enabled)
        return false;

    bool hasOverflow = false;

    if (m_nbClocks % m_timerControlValue == 0)
    {
        if (++m_timerCounter == 0x00) // overflow after increment
        {
            hasOverflow = true;
            m_timerCounter = m_timerModulo; // reset counter to the modulo value
        }
    }

    return hasOverflow;
}

void Timer::Reset()
{
    m_divider = 0x00;
    m_timerCounter = 0x00;
    m_timerModulo = 0x00;
    m_timerControl = 0x00;
    m_enabled = false;
    m_timerControlValue = 4096; // value at 0
    m_nbClocks = 0;
}

void Timer::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    visitor.WriteValue(m_divider);
    visitor.WriteValue(m_timerCounter);
    visitor.WriteValue(m_timerModulo);
    visitor.WriteValue(m_timerControl);
    visitor.WriteValue(m_timerControlValue);
    visitor.WriteValue(m_enabled);
    visitor.WriteValue(m_nbClocks);
}

void Timer::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    visitor.ReadValue(m_divider);
    visitor.ReadValue(m_timerCounter);
    visitor.ReadValue(m_timerModulo);
    visitor.ReadValue(m_timerControl);
    visitor.ReadValue(m_timerControlValue);
    visitor.ReadValue(m_enabled);
    visitor.ReadValue(m_nbClocks);
}