#pragma once

#include <cstdint>

namespace GBEmulator 
{
    class Controller
    {
    public:
        union Buttons
        {
            struct
            {
                uint8_t Right: 1;
                uint8_t Left: 1;
                uint8_t Down: 1;
                uint8_t Up: 1;
                uint8_t Start: 1;
                uint8_t Select: 1;
                uint8_t B: 1;
                uint8_t A: 1;
            };
            uint8_t reg = 0x00;
        };

        Controller() = default;
        virtual ~Controller() = default;

        void ToggleA(bool value) { m_buttons.A = value; }
        void ToggleB(bool value) { m_buttons.B = value; }
        void ToggleSelect(bool value) { m_buttons.Select = value; }
        void ToggleStart(bool value) { m_buttons.Start = value; }
        void ToggleUp(bool value) { m_buttons.Up = value; }
        void ToggleDown(bool value) { m_buttons.Down = value; }
        void ToggleLeft(bool value) { m_buttons.Left = value; }
        void ToggleRight(bool value) { m_buttons.Right = value; }

        uint8_t GetButtonsState() const { return m_buttons.reg; }

    protected:
        Buttons m_buttons;
    };
}