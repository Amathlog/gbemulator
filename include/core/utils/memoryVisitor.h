#include <core/utils/visitor.h>

namespace GBEmulator
{
namespace Utils
{

class MemoryReadVisitor : public IReadVisitor
{
public:
    MemoryReadVisitor(const uint8_t* data, size_t size);

    void Read(uint8_t* data, size_t size) override;
    void Peek(uint8_t* data, size_t size) override;
    size_t Remaining() const override;
    void Advance(size_t size) override;

private:
    const uint8_t* m_data;
    size_t m_ptr;
    size_t m_size;
};


class MemoryWriteVisitor : public IWriteVisitor
{
public:
    MemoryWriteVisitor(uint8_t* data, size_t size);

    void Write(const uint8_t* data, size_t size) override;
    size_t Written() const override;

private:
    uint8_t* m_data;
    size_t m_ptr;
    size_t m_size;
};

}
}