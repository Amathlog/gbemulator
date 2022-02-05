#pragma once

#include <core/utils/visitor.h>

namespace GBEmulator
{
    namespace Utils 
    {
        class IWriteVisitor;
        class IReadVisitor;
    }

    class ISerializable
    {
    public:
        virtual ~ISerializable() = default;
        virtual void SerializeTo(Utils::IWriteVisitor& visitor) const = 0;
        virtual void DeserializeFrom(Utils::IReadVisitor& visitor) = 0;
    };
}