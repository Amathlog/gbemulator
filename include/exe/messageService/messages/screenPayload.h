#pragma once

#include <exe/messageService/message.h>
#include <exe/rendering/image.h>
#include <cstdint>

namespace GBEmulatorExe
{
    using ScreenMessageType = uint32_t;

    enum DefaultScreenMessageType : ScreenMessageType
    {
        CHANGE_FORMAT = 0,
        RESIZE,
        GET_FORMAT,
        RENDER,
        GET_FRAMETIME
    };

    class ScreenPayload : public Payload
    {
    public:
        ScreenPayload(ScreenMessageType type, Format format, int width, int height)
            : m_type(type)
            , m_format(format)
            , m_width(width)
            , m_height(height)
        {}

        ScreenPayload(ScreenMessageType type, const void* data, size_t size)
            : m_type(type)
            , m_dataPtr(data)
            , m_dataSize(size)
        {}

        ScreenPayload(ScreenMessageType type)
            : m_type(type)
        {}

        ScreenMessageType m_type;
        Format m_format = Format::UNDEFINED;
        int m_width = 0;
        int m_height = 0;
        // Render and frametimes
        const void* m_dataPtr = nullptr;
        size_t m_dataSize = 0;
        size_t m_offset = 0;
    };
}