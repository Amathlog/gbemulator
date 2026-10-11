#pragma once

#include <array>
#include <cstdint>

namespace GBEmulator
{
namespace Utils
{
template <typename T, std::size_t N>
class RingBuffer
{
public:
    using UnderlyingType = T;

    RingBuffer() = default;
    RingBuffer(const T& defaultValue) { m_buffer.fill(defaultValue); }

    constexpr std::size_t Size() { return N; }

    void Push(T&& value)
    {
        m_ptr = GetIndexWithOffset(1);
        m_buffer[m_ptr] = std::forward<T>(value);
        m_size = m_size == N ? N : m_size + 1;
    }

    void Push(const T& value)
    {
        m_ptr = GetIndexWithOffset(1);
        m_buffer[m_ptr] = value;
        m_size = m_size == N ? N : m_size + 1;
    }

    T Pop()
    {
        std::size_t old_ptr = m_ptr;
        m_ptr = GetIndexWithOffset(-1);
        return std::move(m_buffer[old_ptr]);
    }

    const T& Last() { return m_buffer[m_ptr]; }
    const T& First() { return m_buffer[GetIndexWithOffset(-static_cast<int64_t>(m_size))]; }

    T& operator[](std::size_t index) { return m_buffer[GetIndexWithOffset(index)]; }
    const T& operator[](std::size_t index) const { return m_buffer[GetIndexWithOffset(index)]; }

private:
    static std::size_t GetIndexWithOffset(std::size_t ptr, int64_t n)
    {
        int64_t index = ptr;
        index += n;
        while (index < 0)
        {
            index += N;
        }
        while (index >= N)
        {
            index -= N;
        }
        return index;
    }

    std::size_t GetIndexWithOffset(int64_t n) { return GetIndexWithOffset(m_ptr, n); }

    std::size_t m_ptr = 0;
    std::size_t m_size = 0;
    std::array<T, N> m_buffer;
};
} // namespace Utils
} // namespace GBEmulator