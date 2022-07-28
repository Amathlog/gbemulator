#include <core/controller.h>

using GBEmulator::Controller;

void Controller::WriteData(uint8_t data)
{
    // Action selection if 5th bit is 0.
    // Direction selection if 4th bit is 0.
    switch((data & 0x30) >> 4)
    {
    case 0:
        m_buttonSelection = ButtonSelection::All;
        break;
    case 1:
        m_buttonSelection = ButtonSelection::ActionSelection;
        break;
    case 2:
        m_buttonSelection = ButtonSelection::DirectionSelection;
        break;
    case 3:
    default:
        m_buttonSelection = ButtonSelection::None;
        break;
    }
}

inline bool Controller::IsValidSelection() const
{
    return m_buttonSelection == ButtonSelection::ActionSelection || m_buttonSelection == ButtonSelection::DirectionSelection;
}

uint8_t Controller::ReadData() const
{
    if (!IsValidSelection())
        return 0xFF;

    // Button is "pressed" if the value is 0.
    return m_buttonSelection == ButtonSelection::ActionSelection ? ~(m_buttonsStatus.reg >> 4) : ~(m_buttonsStatus.reg & 0x0F);
}


bool Controller::HasChangedFromHighToLow() const
{
    if (m_buttonSelection != ButtonSelection::ActionSelection && m_buttonSelection != ButtonSelection::DirectionSelection)
        return false;

    return m_buttonsStatus.reg > 0 && m_previousStatus.reg == 0;
}

void Controller::Update()
{
    m_previousStatus = m_buttonsStatus;
}

void Controller::Reset()
{
    m_buttonSelection = ButtonSelection::None;
    m_buttonsStatus.reg = 0x00;
    m_previousStatus.reg = 0x00;
}