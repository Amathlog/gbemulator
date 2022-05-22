#pragma once

#include <exe/rendering/shaders.h>
// #include <core/constants.h>
#include <memory>
#include <vector>
#include <cstdint>
#include <mutex>
#include <chrono>

namespace GBEmulatorExe
{    
    enum class Format : unsigned
    {
        STRETCH = 0,
        ORIGINAL = 1,

        COUNT = 2,
        UNDEFINED = 0xFFFFFFFF
    };

    class Image
    {
    public:

        Image(unsigned widthRes, unsigned heightRes);
        ~Image();

        void Draw();
        bool IsInitialized() const { return m_initialized; }
        bool BufferWasUpdated() const { return m_bufferWasUpdated; }
        void UpdateRatio(int width, int height);
        void SetImageFormat(Format format);
        Format GetImageFormat() const { return m_format; }

        void UpdateInternalBuffer(const uint8_t* data, size_t size);
        void UpdateGLTexture();
        unsigned GetTextureId() const { return m_texture; }

        auto& GetInternalBuffer() { return m_imageBuffer; }

    private:
        bool InitializeImage();

        bool m_initialized = false;

        unsigned m_internalResWidth;
        unsigned m_internalResHeight;

        int m_currentWidth;
        int m_currentHeight;

        Format m_format = Format::ORIGINAL;
        float m_imageFormat[2];

        // Shaders
        Shader m_shader;
        // Image
        unsigned m_VAO;
        unsigned m_VBO;
        unsigned m_EBO;
        unsigned m_texture;

        std::vector<uint8_t> m_imageBuffer;
        bool m_bufferWasUpdated;
        mutable std::mutex m_lock;
    };
}