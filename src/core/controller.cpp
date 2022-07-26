#include <core/controller.h>

using GBEmulator::Controller;

void Controller::WriteData(uint8_t data)
{
    // Action selection if 5th bit is 0.
    // Direction selection if 4th bit is 0.
    // Since we cannot have both at the same time, make the assumption that
    // we only look at the 5th bit. TO BE VERIFIED.
    m_isActionSelection = (data & 0x20) == 0;
}

uint8_t Controller::ReadData() const
{
    // Button is "pressed" if the value is 0.
    return m_isActionSelection ? ~(m_buttonsStatus.reg >> 4) : ~(m_buttonsStatus.reg & 0x0F);
}