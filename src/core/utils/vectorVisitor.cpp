#include <core/utils/vectorVisitor.h>
#include <cstring>
#include <cassert>

using GBEmulator::Utils::VectorWriteVisitor;
using GBEmulator::Utils::VectorReadVisitor;

VectorReadVisitor::VectorReadVisitor(const std::vector<uint8_t>& data)
    : m_data(data)
    , m_ptr(0)
{

}

void VectorReadVisitor::Read(uint8_t* data, size_t size)
{
    Peek(data, size);
    m_ptr += size;
}

void VectorReadVisitor::Peek(uint8_t* data, size_t size)
{
    assert(size <= Remaining());
    memcpy(data, m_data.data() + m_ptr, size);
}

void VectorReadVisitor::Advance(size_t size)
{
    assert(size <= Remaining());
    m_ptr += size;
}

size_t VectorReadVisitor::Remaining() const
{
    return m_data.size() - m_ptr;
}

VectorWriteVisitor::VectorWriteVisitor(std::vector<uint8_t>& data)
    : m_data(data)
{

}

void VectorWriteVisitor::Write(const uint8_t* data, size_t size)
{
    m_data.insert(m_data.end(), data, data + size);
}

size_t VectorWriteVisitor::Written() const
{
    return m_data.size();
}