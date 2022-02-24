#include "core/cartridge.h"
#include "exe/messageService/coreMessageService.h"
#include "exe/messageService/messages/coreMessage.h"
#include <exe/mainWindow.h>
#include <core/bus.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <algorithm>
#include <core/utils/fileVisitor.h>
#include <exe/messageService/messageService.h>
#include <exe/messageService/messages/screenPayload.h>


namespace fs = std::filesystem;
using namespace GBEmulatorExe;

static bool enableAudioByDefault = false;
static bool syncWithAudio = false;
static bool breakOnStart = true;

int main(int argc, char **argv)
{
    // Load a rom from a file
    auto dir = fs::weakly_canonical(fs::path(argv[0])).parent_path();
    auto root = dir / ".." / ".." / "..";

#ifdef WIN32
    root /= "..";
#endif // WIN32

    // Mapper 000 also
    auto path = root / "tests" / "cpu_instrs.gb";

    // path = root / "roms" / "pokemon_jaune.gb";

    // Check the arg, if there is a file to load
    if (argc > 1)
    {
        path = fs::path(argv[1]);
        if (path.is_relative())
            path = root / path;
    }

    GBEmulator::Bus bus;

    if (breakOnStart)
        bus.BreakContinue();

    GBEmulatorExe::CoreMessageService coreMessageService(bus, dir);
    GBEmulatorExe::DispatchMessageServiceSingleton::GetInstance().Connect(&coreMessageService);

    LoadNewGameMessage msg(path.string());
    GBEmulatorExe::DispatchMessageServiceSingleton::GetInstance().Push(msg);

    auto previous_point = std::chrono::high_resolution_clock::now();
    constexpr bool showRealFPS = false;
    constexpr size_t nbSamples = 120;
    //std::array<float, nbSamples> timeCounter;
    size_t ptr = 0;

    {
        MainWindow mainWindow("GB/GBC Emulator", 160*5, 144*5, 160, 144);
        mainWindow.SetUserData(&bus);
        mainWindow.ConnectController();

        // if (audioSystem.Initialize() || !enableAudioByDefault)
        {
            previous_point = std::chrono::high_resolution_clock::now();
            while (!mainWindow.RequestedClose())
            {
                auto start_point = std::chrono::high_resolution_clock::now();
                auto timeSpent = std::chrono::duration_cast<std::chrono::microseconds>(start_point - previous_point).count();
                previous_point = std::chrono::high_resolution_clock::now();
                timeSpent = std::min(timeSpent, 16666l);
                
                double cpuPeriodUS = 1000000.0 / bus.GetCurrentFrequency();
                size_t nbClocks = timeSpent / cpuPeriodUS;
                if (!bus.IsInBreak())
                {
                    for (auto i = 0; i < nbClocks; ++i)
                    {
                        bus.Clock();
                        // if (bus.GetPPU().IsFrameComplete())
                        //     DispatchMessageServiceSingleton::GetInstance().Push(RenderMessage(bus.GetPPU().GetScreen(), bus.GetPPU().GetHeight() * bus.GetPPU().GetWidth()));
                    }
                }

                mainWindow.Update(true);
            }
        }
    }

    return 0;
}