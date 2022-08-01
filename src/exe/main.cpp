#include <algorithm>
#include <core/bus.h>
#include <core/constants.h>
#include <core/cartridge.h>
#include <core/utils/fileVisitor.h>
#include <exe/mainWindow.h>
#include <core/utils/utils.h>
#include <exe/messageService/coreMessageService.h>
#include <exe/messageService/messageService.h>
#include <exe/messageService/messages/coreMessage.h>
#include <exe/messageService/messages/screenMessage.h>
#include <exe/messageService/messages/screenPayload.h>
#include <filesystem>
#include <memory>
#include <vector>

namespace fs = std::filesystem;
using namespace GBEmulatorExe;

static bool enableAudioByDefault = false;
static bool syncWithAudio = false;

static unsigned windowScalingFactor = 5;

int main(int argc, char** argv) {
    // Load a rom from a file
    auto root = GBEmulator::Utils::GetRootPath();
    auto path = std::filesystem::path();

    // Mapper 000 also
    //auto path = root / "tests" / "bgbtest.gb";

    //path = root / "roms" / "SuperMarioLand.gb";

    // Check the arg, if there is a file to load
    if (argc > 1) {
        path = fs::path(argv[1]);
        if (path.is_relative())
            path = root / path;
    }

    GBEmulator::Bus bus;

    GBEmulatorExe::CoreMessageService coreMessageService(bus, GBEmulator::Utils::GetExePath().string());
    GBEmulatorExe::DispatchMessageServiceSingleton::GetInstance().Connect(
        &coreMessageService);

    if (!path.empty())
    {
        LoadNewGameMessage msg(path.string());
        GBEmulatorExe::DispatchMessageServiceSingleton::GetInstance().Push(msg);
    }

    auto previous_point = std::chrono::high_resolution_clock::now();
    constexpr bool showRealFPS = false;
    constexpr size_t nbSamples = 120;
    // std::array<float, nbSamples> timeCounter;
    size_t ptr = 0;

    {
        MainWindow mainWindow("GB/GBC Emulator", 
            GBEmulator::GB_INTERNAL_WIDTH * windowScalingFactor,
            GBEmulator::GB_INTERNAL_HEIGHT * windowScalingFactor, 
            GBEmulator::GB_INTERNAL_WIDTH, 
            GBEmulator::GB_INTERNAL_HEIGHT);

        mainWindow.SetUserData(&bus);
        mainWindow.ConnectController();

        // if (audioSystem.Initialize() || !enableAudioByDefault)
        {
            previous_point = std::chrono::high_resolution_clock::now();
            while (!mainWindow.RequestedClose()) {
                auto start_point = std::chrono::high_resolution_clock::now();
                auto timeSpent = std::chrono::duration_cast<std::chrono::microseconds>(
                    start_point - previous_point)
                    .count();
                previous_point = std::chrono::high_resolution_clock::now();
                timeSpent = std::min<int64_t>(timeSpent, 16666ll);
                
                double cpuPeriodUS = 1000000.0 / bus.GetCurrentFrequency();
                size_t nbClocks = (size_t)(timeSpent / cpuPeriodUS);
                if (!bus.IsInBreak()) {
                    for (auto i = 0; i < nbClocks; ++i) {
                        bus.Clock();
                        if (bus.GetPPU().IsFrameComplete())
                             DispatchMessageServiceSingleton::GetInstance().Push(RenderMessage(bus.GetPPU().GetScreen().data(),
                             bus.GetPPU().GetScreen().size()));

                        if (bus.IsInBreak())
                            break;
                    }
                }

                mainWindow.Update(true);
            }
        }
    }

    return 0;
}