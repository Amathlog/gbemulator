#pragma once

#include <cstdint>

namespace GBEmulator 
{
    class Controller
    {
    public:
        enum ButtonSelection : uint8_t
        {
            None = 0,
            ActionSelection = 1,
            DirectionSelection = 2,
            All = ActionSelection | DirectionSelection
        };

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

        bool HasChangedFromHighToLow() const;
        void Update();
        void Reset();

        uint8_t GetButtonsStatus() const { return m_buttonsStatus.reg; }

    protected:
        bool IsValidSelection() const;
        ButtonsStatus m_buttonsStatus;
        ButtonsStatus m_previousStatus;
        ButtonSelection m_buttonSelection = ButtonSelection::None;
    };
}