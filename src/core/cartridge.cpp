#include <algorithm>
#include <cassert>
#include <core/cartridge.h>
#include <core/mappers/mapperBase.h>
#include <core/mappers/mapperFactory.h>
#include <core/utils/sha1.h>
#include <cstdint>
#include <cstring>
#include <sys/types.h>

using GBEmulator::Cartridge;
using GBEmulator::Header;

void Header::FillFromData(const uint8_t* data)
{
    // Nintendo Logo
    std::memcpy(nintendoLogo.data(), data + 0x0104, 0x30);

    // Title
    // 16 bytes in old cartridge, but was reduced to 15 and then 11 with CGB
    std::memcpy(title, data + 0x0134, 0x10);
    title[0x10] = '\0';

    // Manufacturer code (CGB cartridge era only)
    std::memcpy(manufacturerCode, data + 0x013F, 4);
    manufacturerCode[4] = '\0';

    uint8_t cgbFlag = data[0x0143];
    CGBOnly = cgbFlag == 0xC0;
    supportCGBMode = (cgbFlag == 0x80) || supportCGBMode;

    sgbFlag = data[0x0146];
    cartridgeType = data[0x0147];

    uint8_t romSizeCode = data[0x0148];
    assert(romSizeCode <= 8 && "Unsupported rom size");
    nbRomBanks = 1 << (romSizeCode + 1);

    uint8_t ramSizeCode = data[0x0149];
    assert(ramSizeCode <= 5 && "Unsupported ram size");
    switch (ramSizeCode)
    {
    case 0: // Fall-through
    case 1:
        nbRamBanks = 0;
        break;
    case 2:
        nbRamBanks = 1;
        break;
    case 3:
        nbRamBanks = 4;
        break;
    case 4:
        nbRamBanks = 16;
        break;
    case 5:
        nbRamBanks = 8;
        break;
    }

    isJapaneseGame = data[0x014A] == 0;

    // Licensee code
    licenseeCode = data[0x014B];
    // In this case it is the new code
    if (licenseeCode == 0x33)
    {
        uint8_t msb = data[0x0144];
        uint8_t lsb = data[0x0145];
        licenseeCode = ((msb & 0x0F) << 8) | (lsb & 0x0F);
    }

    maskRomVersionNumber = data[0x14C];

    headerChecksum = data[0x014D];
    globalChecksum = ((uint16_t)data[0x014E] << 8) | data[0x014F];
}

Cartridge::Cartridge(Utils::IReadVisitor& visitor)
{
    // First resize the prg data to the size of one bank,
    // and bank 0 data here
    m_prgData.resize(0x4000);

    visitor.Read(m_prgData.data(), 0x4000);

    // From this, extract the header
    m_header.FillFromData(m_prgData.data());

    // Create the mapper
    m_mapper = CreateMapper(m_header);
    if (!m_mapper)
        return;

    // Resize everything
    m_prgData.resize(0x4000 * m_header.nbRomBanks);
    m_externalRAM.resize(m_mapper->GetRAMSize());

    // Read the rest of data
    visitor.Read(m_prgData.data() + 0x4000, 0x4000 * (m_header.nbRomBanks - 1));

    // When all is done, compute the SHA1 of the ROM
    Utils::SHA1 sha1;
    sha1.update(m_prgData);
    m_sha1 = sha1.final();
}

Cartridge::~Cartridge() { delete m_mapper; }

void Cartridge::SerializeRam(Utils::IWriteVisitor& visitor) const { visitor.WriteContainer(m_externalRAM); }

void Cartridge::SerializeTo(Utils::IWriteVisitor& visitor) const
{
    SerializeRam(visitor);

    if (m_mapper)
        m_mapper->SerializeTo(visitor);
}

void Cartridge::DeserializeRam(Utils::IReadVisitor& visitor) { visitor.ReadContainer(m_externalRAM); }

void Cartridge::DeserializeFrom(Utils::IReadVisitor& visitor)
{
    DeserializeRam(visitor);

    if (m_mapper)
        m_mapper->DeserializeFrom(visitor);
}

void Cartridge::Reset()
{
    // This removes all saved data on reset, so disable it for now.
    // std::fill(m_externalRAM.begin(), m_externalRAM.end(), 0x00);

    if (m_mapper)
        m_mapper->Reset();
}

bool Cartridge::ReadByte(uint16_t addr, uint8_t& data, bool /*readOnly*/)
{
    // No mapper, nothing to do
    if (m_mapper == nullptr)
        return false;

    // If we have a custom read
    if (m_mapper->HasCustomReadWrite(addr) && m_mapper->ReadByte(addr, data))
    {
        return true;
    }

    // ROM zone
    if (addr < 0x8000)
    {
        // Compute the bank we need. Between 0x0000 and 0x3FFF it the first bank (usually fixed at 0)
        // between 0x4000 and 0x7FFF it is the switchable one.
        uint32_t prgDataBank = (addr & 0x4000) ? m_mapper->GetSecondROMBank() : m_mapper->GetFirstROMBank();
        // ROM Banks are 16kB in size.
        data = m_prgData[prgDataBank * 0x4000 + (addr & 0x3FFF)];
        return true;
    }
    // RAM zone
    else if (addr >= 0xA000 && addr < 0xC000 && m_mapper->IsRamEnabled())
    {
        // RAM banks are 8kB in size usually. In case of a smaller RAM, we cut the address
        if (m_externalRAM.size() >= 0x2000)
        {
            data = m_externalRAM[m_mapper->GetRAMBank() * 0x2000 + (addr & 0x1FFF)];
        }
        else
        {
            data = m_externalRAM[addr & (uint16_t)(m_externalRAM.size())];
        }
        return true;
    }

    return false;
}

bool Cartridge::WriteByte(uint16_t addr, uint8_t data)
{
    // No mapper, nothing to do
    if (m_mapper == nullptr)
        return false;

    // If we have a custom write
    if (m_mapper->HasCustomReadWrite(addr) && m_mapper->WriteByte(addr, data))
    {
        return true;
    }

    // Try to write to the RAM
    if (addr >= 0xA000 && addr < 0xC000 && m_mapper->IsRamEnabled())
    {
        // RAM banks are 8kB in size usually. In case of a smaller RAM, we cut the address
        if (m_externalRAM.size() >= 0x2000)
        {
            m_externalRAM[m_mapper->GetRAMBank() * 0x2000 + (addr & 0x1FFF)] = data;
        }
        else
        {
            m_externalRAM[addr & (uint16_t)(m_externalRAM.size())] = data;
        }
    }

    // Otherwise, send the data to the mapper
    return m_mapper->WriteByte(addr, data);
}