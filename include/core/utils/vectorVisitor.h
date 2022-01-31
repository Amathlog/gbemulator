#include <core/utils/visitor.h>
#include <vector>

namespace GBEmulator
{
namespace Utils
{

class VectorReadVisitor : public IReadVisitor
{
public:
    VectorReadVisitor(const std::vector<uint8_t>& data);

    void Read(uint8_t* data, size_t size) override;
    void Peek(uint8_t* data, size_t size) override;
    size_t Remaining() const override;
    void Advance(size_t size) override;

private:
    const std::vector<uint8_t>& m_data;
    size_t m_ptr;
};


class VectorWriteVisitor : public IWriteVisitor
{
public:
    VectorWriteVisitor(std::vector<uint8_t>& data);

    void Write(const uint8_t* data, size_t size) override;
    size_t Written() const override;

private:
    std::vector<uint8_t>& m_data;
};

}
}