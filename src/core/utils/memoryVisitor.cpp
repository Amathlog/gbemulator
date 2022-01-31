#include <core/utils/memoryVisitor.h>
#include <cstring>
#include <cassert>

using GBEmulator::Utils::MemoryReadVisitor;
using GBEmulator::Utils::MemoryWriteVisitor;

MemoryReadVisitor::MemoryReadVisitor(const uint8_t* data, size_t size)
    : m_data(data)
    , m_size(size)
    , m_ptr(0)
{

}

void MemoryReadVisitor::Read(uint8_t* data, size_t size)
{
    Peek(data, size);
    m_ptr += size;
}

void MemoryReadVisitor::Peek(uint8_t* data, size_t size)
{
    assert(size <= Remaining());
    memcpy(data, m_data + m_ptr, size);
}

void MemoryReadVisitor::Advance(size_t size)
{
    assert(size <= Remaining());
    m_ptr += size;
}

size_t MemoryReadVisitor::Remaining() const
{
    return m_size - m_ptr;
}

MemoryWriteVisitor::MemoryWriteVisitor(uint8_t* data, size_t size)
    : m_data(data)
    , m_size(size)
    , m_ptr(0)
{

}

void MemoryWriteVisitor::Write(const uint8_t* data, size_t size)
{
    assert(size <= (m_size - m_ptr));
    memcpy(m_data + m_ptr, data, size);
    m_ptr += size;
}

size_t MemoryWriteVisitor::Written() const
{
    return m_ptr;
}