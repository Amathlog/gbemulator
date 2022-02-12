#pragma once

#include <exe/messageService/messageService.h>
#include <vector>
#include <string>

namespace GBEmulator 
{
    class Bus;
}

namespace GBEmulatorExe
{
    class CoreMessageService : public IMessageService
    {
    public:
        CoreMessageService(GBEmulator::Bus& bus, std::string exePath) 
            : m_bus(bus)
            , m_exePath(exePath)
        {}

        bool Push(const Message& message) override;
        bool Pull(Message& message) override;

    private:
        // Handlers
        bool LoadNewGame(const std::string& file);
        bool LoadSaveGame(const std::string& file);
        bool SaveGame(const std::string& file);
        bool SaveState(const std::string& file, int number);
        bool LoadState(const std::string& file, int number);

        GBEmulator::Bus& m_bus;
        std::string m_exePath;

        // Temporary
        std::vector<uint8_t> m_stateData;
    };
}