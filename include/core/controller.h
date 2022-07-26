#pragma once

#include <cstdint>

namespace GBEmulator 
{
    class Controller
    {
    public:
        union ButtonsStatus
        {
            struct
            {
                uint8_t Right: 1;
                uint8_t Left: 1;
                uint8_t Up: 1;
                uint8_t Down: 1;
                uint8_t A: 1;
                uint8_t B: 1;
                uint8_t Select: 1;
                uint8_t Start: 1;
            };
            uint8_t reg = 0x00;
        };

        Controller() = default;
        virtual ~Controller() = default;

        void ToggleA(bool value) { m_buttonsStatus.A = value; }
        void ToggleB(bool value) { m_buttonsStatus.B = value; }
        void ToggleSelect(bool value) { m_buttonsStatus.Select = value; }
        void ToggleStart(bool value) { m_buttonsStatus.Start = value; }
        void ToggleUp(bool value) { m_buttonsStatus.Up = value; }
        void ToggleDown(bool value) { m_buttonsStatus.Down = value; }
        void ToggleLeft(bool value) { m_buttonsStatus.Left = value; }
        void ToggleRight(bool value) { m_buttonsStatus.Right = value; }

        void WriteData(uint8_t data);
        uint8_t ReadData() const;

        uint8_t GetButtonsStatus() const { return m_buttonsStatus.reg; }

    protected:
        ButtonsStatus m_buttonsStatus;
        bool m_isActionSelection = false;
    };
}