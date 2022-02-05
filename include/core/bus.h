#pragma once

#include <core/serializable.h>
#include <core/z80Processor.h>

namespace GBEmulator 
{
    class Bus : public ISerializable
    {
    public:
        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        void Reset();
    private:
        Z80Processor m_cpu;
    };
}