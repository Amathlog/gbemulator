#pragma once

#include <core/utils/visitor.h>
#include <string>
#include <fstream>

namespace GBEmulator
{
namespace Utils
{

enum class FileVersion : int32_t
{
    NoVersion = 0,
    Version_0_0_1 = 1,
    Version_0_1_0 = 1 << 8,
    Version_1_0_0 = 1 << 16,
    
    CurrentVersion = Version_0_0_1,
};

class FileReadVisitor : public IReadVisitor
{
public:
    FileReadVisitor(const std::string& file, bool withVersioning = false);
    ~FileReadVisitor();

    void Read(uint8_t* data, size_t size) override;
    void Peek(uint8_t* data, size_t size) override;
    size_t Remaining() const override;
    void Advance(size_t size) override;

    bool IsValid() const { return m_file.is_open(); }
    FileVersion GetVersion() const { return m_version; }

private:
    std::ifstream m_file;
    size_t m_ptr;
    size_t m_size;
    FileVersion m_version;
};


class FileWriteVisitor : public IWriteVisitor
{
public:
    FileWriteVisitor(const std::string& file, bool withVersioning = false);
    ~FileWriteVisitor();

    void Write(const uint8_t* data, size_t size) override;
    size_t Written() const override;

    bool IsValid() const { return m_file.is_open(); }
    FileVersion GetVersion() const { return m_version; }

private:
    std::ofstream m_file;
    size_t m_ptr;
    FileVersion m_version;
};


}
}