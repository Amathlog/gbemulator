#pragma once

#include <atomic>
#include <cstring>
#include <iostream>
#include <mutex>
#include <vector>

namespace GBEmulator
{
template <typename T>
class CircularBuffer
{
public:
    CircularBuffer(size_t maxSize)
        : m_maxSize(maxSize)
    {
        m_buffer.resize(maxSize);
    }

    ~CircularBuffer() = default;

    void ReadData(void* outData, size_t bytesSize)
    {
        constexpr bool fillMissingWith0 = true;

        size_t nbElements = bytesSize / sizeof(T);
        bool said = false;

        if constexpr (!fillMissingWith0)
        {
            while (!m_stop && nbElements > m_nbReadableSamples)
            {
                if (!said)
                {
                    // std::cout << "Reader too fast" << std::endl;
                    said = false;
                }
                // Waiting
            }
        }

        if (m_stop)
        {
            std::memset(outData, 0, sizeof(T) * nbElements);
        }

        std::scoped_lock lock(m_lock);
        if constexpr (fillMissingWith0)
        {
            size_t missingSamples = nbElements > m_nbReadableSamples ? nbElements - m_nbReadableSamples : 0;
            nbElements -= missingSamples;
            std::memset((char*)outData + (nbElements * sizeof(T)), 0, sizeof(T) * missingSamples);
        }
        size_t targetPtr = m_readerPtr + nbElements;

        if (targetPtr >= m_maxSize)
        {
            size_t tempNbElements = (m_maxSize - m_readerPtr);
            size_t tempSize = tempNbElements * sizeof(T);
            std::memcpy(outData, m_buffer.data() + m_readerPtr, tempSize);
            targetPtr -= m_maxSize;
            outData = ((char*)outData) + tempSize;
            bytesSize -= tempSize;
            m_readerPtr = 0;
            m_nbReadableSamples -= tempNbElements;
            nbElements -= tempNbElements;
        }

        std::memcpy(outData, m_buffer.data() + m_readerPtr, bytesSize);
        m_nbReadableSamples -= nbElements;
        m_readerPtr += nbElements;
    }

    void WriteData(const void* inData, size_t bytesSize)
    {
        size_t nbElements = bytesSize / sizeof(T);

        while (!m_stop && nbElements + m_nbReadableSamples > m_maxSize)
        {
            // Waiting
        }

        if (m_stop)
        {
            return;
        }

        std::scoped_lock lock(m_lock);
        size_t targetPtr = m_writerPtr + nbElements;

        if (targetPtr >= m_maxSize)
        {
            size_t tempNbElements = (m_maxSize - m_writerPtr);
            size_t tempSize = tempNbElements * sizeof(T);
            std::memcpy(m_buffer.data() + m_writerPtr, inData, tempSize);
            targetPtr -= m_maxSize;
            inData = ((const char*)inData) + tempSize;
            bytesSize -= tempSize;
            m_writerPtr = 0;
            m_nbReadableSamples += tempNbElements;
            nbElements -= tempNbElements;
        }

        std::memcpy(m_buffer.data() + m_writerPtr, inData, bytesSize);
        m_writerPtr += nbElements;
        m_nbReadableSamples += nbElements;

        if (m_nbReadableSamples > m_maxSize)
        {
            std::cout << "Writer is writing faster than the reader reads" << std::endl;
        }
    }

    void Stop() { m_stop = true; }

    void Reset()
    {
        m_stop = false;
        m_nbReadableSamples = 0;
        m_writerPtr = 0;
        m_readerPtr = 0;
    }

private:
    std::mutex m_lock;
    size_t m_maxSize;
    std::vector<T> m_buffer;

    std::atomic<bool> m_stop = false;
    std::atomic<size_t> m_nbReadableSamples = 0;

    size_t m_writerPtr = 0;
    size_t m_readerPtr = 0;
};
} // namespace GBEmulator