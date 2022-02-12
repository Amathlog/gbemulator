#include <exe/rendering/image.h>
#include <exe/rendering/defaultShadersPrg.h>
#include <glad/glad.h>
// #include <core/palette.h>
#include <cstring>
#include <cassert>

using GBEmulatorExe::Image;

Image::Image(unsigned widthRes, unsigned heightRes)
    : m_internalResWidth(widthRes)
    , m_internalResHeight(heightRes)
    , m_shader(vertexShaderPrg, fragmentShaderPrg)
    , m_initialized(false)
    , m_currentWidth(widthRes)
    , m_currentHeight(heightRes)
{
    if (!InitializeImage())
        return;

    m_initialized = true;

    m_format = Format::STRETCH;
    m_imageFormat[0] = 1.0f;
    m_imageFormat[1] = 1.0f;

    m_imageBuffer.resize(m_internalResWidth * m_internalResHeight);
}

Image::~Image()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
    glDeleteTextures(1, &m_texture);
}

void Image::Draw()
{
    if (!m_initialized)
        return;

    int paletteLocation = glGetUniformLocation(m_shader.GetProgramId(), "palette");
    // glUniform3fv(paletteLocation, 64, GBEmulator::Palette::GetPaletteFloat());

    int screenFormatLocation = glad_glGetUniformLocation(m_shader.GetProgramId(), "screenFormat");
    glUniform2fv(screenFormatLocation, 1, m_imageFormat);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    m_shader.UseProgram();

    if (m_bufferWasUpdated)
    {
        UpdateGLTexture();
    }

    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void Image::UpdateGLTexture()
{
    std::unique_lock<std::mutex> lk(m_lock);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_internalResWidth, m_internalResHeight, GL_RED, GL_UNSIGNED_BYTE, m_imageBuffer.data());
    m_bufferWasUpdated = false;
}

void Image::UpdateInternalBuffer(const uint8_t* data, size_t size)
{
    std::unique_lock<std::mutex> lk(m_lock);
    assert(data != nullptr && size == m_imageBuffer.size());
    std::memcpy(m_imageBuffer.data(), data, m_imageBuffer.size());
    m_bufferWasUpdated = true;
}


void Image::SetImageFormat(Format format)
{
    m_format = format;
    UpdateRatio(m_currentWidth, m_currentHeight);
}

void Image::UpdateRatio(int width, int height)
{
    m_currentWidth = width;
    m_currentHeight = height;

    float currentRatio = (float) width / height;
    float targetRatio = 1.0f;

    switch(m_format)
    {
    case Format::STRETCH:
        targetRatio = currentRatio;
        break;
    case Format::ORIGINAL:
    {
        targetRatio = (float)m_internalResWidth / m_internalResHeight;
        break;
    }
    default:
        return;
    }

    if (currentRatio > targetRatio)
    {
        // We have a bigger width that needed
        m_imageFormat[0] = targetRatio / currentRatio;
        m_imageFormat[1] = 1.0f;
    }
    else 
    {
        // We have a bigger height that needed
        m_imageFormat[0] = 1.0f;
        m_imageFormat[1] = currentRatio / targetRatio;
    }
}

bool Image::InitializeImage()
{
        float vertices[] = {
        1.0f, 1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 0.0f,
    };

    unsigned indices[] = {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, m_internalResWidth, m_internalResHeight, 0, GL_RED, GL_UNSIGNED_BYTE, (void*)0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    m_shader.UseProgram();
    glUniform1i(glGetUniformLocation(m_shader.GetProgramId(), "Texture"), 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}