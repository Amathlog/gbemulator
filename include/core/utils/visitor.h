#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <queue>
#include <array>

using std::size_t;

namespace GBEmulator
{
    namespace Utils
    {
        template<typename T>
        struct IsStdArray : std::false_type {};

        template<typename T, std::size_t N>
        struct IsStdArray<std::array<T, N>> : std::true_type {};

        class IReadVisitor
        {
        public:
            virtual ~IReadVisitor() = default;

            virtual void Read(uint8_t* data, size_t size) = 0;
            virtual void Peek(uint8_t* data, size_t size) = 0;

            template<typename T>
            void ReadValue(T& data)
            {
                Read(&data, 1);
            }

            template<typename T>
            void Read(T* data, size_t size)
            {
                Read(reinterpret_cast<uint8_t*>(data), size * sizeof(T));
            }

            template<typename T>
            void Peek(T* data, size_t size)
            {
                Peek(reinterpret_cast<uint8_t*>(data), size * sizeof(T));
            }

            template<typename T>
            void ReadAll(T* data)
            {
                Read(data, Remaining() / sizeof(T));
            }

            template<typename T>
            void PeekAll(T* data)
            {
                Read(data, Remaining() / sizeof(T));
            }

            template<typename Container>
            void ReadContainer(Container& data)
            {
                size_t size = 0;
                ReadValue(size);
                if constexpr (!IsStdArray<Container>::value)
                {
                    data.resize(size);
                }
                else
                {
                    assert(size == data.size());
                }

                Read(data.data(), size);
            }

            template <typename DataType, typename Container>
            void ReadQueue(std::queue<DataType, Container>& data)
            {
                // First make sure the queue is empty
                while (!data.empty())
                    data.pop();

                size_t size = 0;
                ReadValue(size);
                for (size_t i = 0; i < size; ++i)
                {
                    DataType value;
                    ReadValue(value);
                    data.push(std::move(value));
                }
            }

            virtual void Advance(size_t size) = 0;
            
            virtual size_t Remaining() const = 0;
        };

        class IWriteVisitor
        {
        public:
            virtual ~IWriteVisitor() = default;

            virtual void Write(const uint8_t* data, size_t size) = 0;

            template <typename T>
            void WriteValue(const T& data)
            {
                Write(&data, 1);
            }

            template <typename T>
            void Write(const T* data, size_t size)
            {
                Write(reinterpret_cast<const uint8_t*>(data), size * sizeof(T));
            }

            template <typename Container>
            void WriteContainer(const Container& data)
            {
                WriteValue(data.size());
                Write(data.data(), data.size());
            }

            template <typename DataType, typename Container>
            void WriteQueue(std::queue<DataType, Container>& data)
            {
                WriteValue(data.size());
                while (!data.empty())
                {
                    WriteValue(data.front());
                    data.pop();
                }
            }

            virtual size_t Written() const = 0;
        };
    }
}