#include <core/utils/fileVisitor.h>
#include <cstring>
#include <cassert>
#include <limits>
#include <iostream>

using GBEmulator::Utils::FileReadVisitor;
using GBEmulator::Utils::FileWriteVisitor;

template <typename T>
void Open(T& stream, const std::string& filename, bool open, bool binary)
{
    std::ios_base::openmode mode = open ? std::ios_base::in : std::ios_base::out;
    if (binary)
        mode |= std::ios_base::binary;

    stream.open(filename, mode);
}

template <typename T>
void Close(T& stream)
{
    stream.close();
}

FileReadVisitor::FileReadVisitor(const std::string& file, bool withVersioning, bool binary)
    : m_ptr(0)
{
    Open(m_file, file, true, binary);

    m_size = 0;

    // Gets its size
    if (!m_file.is_open())
    {
        std::cerr << "Failed to open file " << file << std::endl;
        return;
    }

    m_file.ignore( std::numeric_limits<std::streamsize>::max() );
    m_size = (size_t)m_file.gcount();
    m_file.clear();   //  Since ignore will have set eof.
    m_file.seekg( 0, std::ios_base::beg );
    
    if (withVersioning)
        ReadValue(m_version);
    else
        m_version = FileVersion::NoVersion;
}

FileReadVisitor::~FileReadVisitor()
{
    Close(m_file);
}

void FileReadVisitor::Read(uint8_t* data, size_t size)
{
    assert(size <= Remaining());
    m_file.read(reinterpret_cast<char*>(data), size);
    m_ptr += size;
}

void FileReadVisitor::Peek(uint8_t* data, size_t size)
{
    assert(size <= Remaining());
    m_file.read(reinterpret_cast<char*>(data), size);
    // Go back in the stream
    m_file.seekg(std::streamoff(-static_cast<long long>(size)), std::ios_base::cur);
}

void FileReadVisitor::Advance(size_t size)
{
    assert(size <= Remaining());
    m_file.seekg(size, std::ios_base::cur);
    m_ptr += size;
}

size_t FileReadVisitor::Remaining() const
{
    return m_size - m_ptr;
}

FileWriteVisitor::FileWriteVisitor(const std::string& file, bool withVersioning, bool binary)
    : m_ptr(0)
{
    Open(m_file, file, false, binary);
    m_version = FileVersion::CurrentVersion;

    if (withVersioning)
        WriteValue(m_version);
    else
        m_version = FileVersion::NoVersion;
}

FileWriteVisitor::~FileWriteVisitor()
{
    Close(m_file);
}

void FileWriteVisitor::Write(const uint8_t* data, size_t size)
{
    assert(m_file.is_open());
    m_file.write(reinterpret_cast<const char*>(data), size);
    m_ptr += size;
}

size_t FileWriteVisitor::Written() const
{
    return m_ptr;
}