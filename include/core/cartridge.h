#pragma once

#include <core/serializable.h>
#include <core/utils/visitor.h>
#include <vector>
#include <cstdint>
#include <array>

namespace GBEmulator
{
    struct Header
    {
        std::array<uint8_t, 0x30> nintendoLogo;
        char title[17];
        char manufacturerCode[5];
        bool supportCGBMode;
        bool CGBOnly;
        uint16_t licenseeCode;
        uint8_t sgbFlag;
        uint8_t cartridgeType;
        uint8_t nbRomBanks;
        uint8_t nbRamBanks;
        bool isJapaneseGame;
        uint8_t maskRomVersionNumber;
        uint8_t headerChecksum;
        uint16_t globalChecksum;

        void FillFromData(const uint8_t* data);
    };

    class Cartridge : public ISerializable
    {
    public:
        Cartridge(Utils::IReadVisitor& visitor);

        void SerializeTo(Utils::IWriteVisitor& visitor) const override;
        void DeserializeFrom(Utils::IReadVisitor& visitor) override;

        // Read a single byte of data, passed as an out argument
        // Will return true if the cartridge has provided some data
        bool ReadByte(uint16_t addr, uint8_t& data);

        // Write a single byte of data
        // Will return true if the cartridge has done a write operation
        bool WriteByte(uint16_t addr, uint8_t data);

        void Reset();

        const Header& GetHeader() const { return m_header; }
        const std::string& GetSHA1() const { return m_sha1; }

    private:
        Header m_header;

        std::string m_sha1;

        std::vector<uint8_t> m_externalRAM;
        uint8_t m_currentExternalRAMBank = 0;

        std::vector<uint8_t> m_prgData;
        uint8_t m_currentPrgDataBank;
    };
}