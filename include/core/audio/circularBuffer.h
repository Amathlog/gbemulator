#pragma once

#include <mutex>
#include <vector>
#include <atomic>
#include <cstring>

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

        void ReadData(void* outData, size_t bytesSize)
        {
            size_t nbElements = bytesSize / sizeof(T);
            bool said = false;

            while (!m_stop && nbElements > m_nbReadableSamples)
            {
                if (!said)
                {
                    //std::cout << "Reader too fast" << std::endl;
                    said = false;
                }
                // Waiting
            }

            if (m_stop)
            {
                return;
            }

            std::scoped_lock lock(m_lock);
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

    private:
        mutable std::mutex m_lock;
        size_t m_maxSize;
        std::vector<T> m_buffer;

        std::atomic<bool> m_stop = false;
        std::atomic<size_t> m_nbReadableSamples = 0;

        size_t m_writerPtr = 0;
        size_t m_readerPtr = 0;
    };
}