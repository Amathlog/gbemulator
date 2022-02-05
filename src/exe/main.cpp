#include <exe/mainWindow.h>
#include <core/bus.h>
#include <filesystem>
#include <vector>
#include <algorithm>


namespace fs = std::filesystem;
using namespace GBEmulatorExe;

static bool enableAudioByDefault = true;
static bool syncWithAudio = false;

int main(int argc, char **argv)
{
    // Load a rom from a file
    auto dir = fs::weakly_canonical(fs::path(argv[0])).parent_path();
    auto root = dir / ".." / ".." / "..";

#ifdef WIN32
    root /= "..";
#endif // WIN32

    // Mapper 000 also
    auto path = root / "tests" / "test_roms" / "nestest.nes";

    // path = root / "roms" / "smb.nes";

    // Check the arg, if there is a file to load
    if (argc > 1)
    {
        path = fs::path(argv[1]);
        if (path.is_relative())
            path = root / path;
    }

    GBEmulator::Bus bus;

    auto previous_point = std::chrono::high_resolution_clock::now();
    constexpr bool showRealFPS = false;
    constexpr size_t nbSamples = 120;
    std::array<float, nbSamples> timeCounter;
    size_t ptr = 0;

    {
        MainWindow mainWindow("GB/GBC Emulator", 160*5, 144*5, 160, 144);
        // mainWindow.SetUserData(&bus);
        mainWindow.ConnectController();

        // if (audioSystem.Initialize() || !enableAudioByDefault)
        {
            previous_point = std::chrono::high_resolution_clock::now();
            while (!mainWindow.RequestedClose())
            {
                if (!syncWithAudio)
                {
                    // do {
                    //     bus.Clock();
                    // } while (!bus.GetPPU().IsFrameComplete());
                    // DispatchMessageServiceSingleton::GetInstance().Push(RenderMessage(bus.GetPPU().GetScreen(), bus.GetPPU().GetHeight() * bus.GetPPU().GetWidth()));


                //     auto start_point = std::chrono::high_resolution_clock::now();
                //     auto timeSpent = std::chrono::duration_cast<std::chrono::microseconds>(start_point - previous_point).count();
                //     previous_point = std::chrono::high_resolution_clock::now();
                //     timeSpent = std::min(timeSpent, 16666l);
                    
                //     constexpr double ppuPeriodUS = 1000000.0 / NesEmulator::Cst::NTSC_PPU_FREQUENCY;
                //     size_t nbClocks = timeSpent / ppuPeriodUS;
                //     for (auto i = 0; i < nbClocks; ++i)
                //     {
                //         bus.Clock();
                //         if (bus.GetPPU().IsFrameComplete())
                //             DispatchMessageServiceSingleton::GetInstance().Push(RenderMessage(bus.GetPPU().GetScreen(), bus.GetPPU().GetHeight() * bus.GetPPU().GetWidth()));
                //     }

                //     if constexpr (showRealFPS)
                //     {
                //         auto end_point = std::chrono::high_resolution_clock::now();
                //         timeSpent = std::chrono::duration_cast<std::chrono::microseconds>(end_point - start_point).count();
                //         double ratio = (double)(timeSpent) / (ppuPeriodUS * nbClocks); // < 1 = faster than realtime
                //         timeCounter[ptr++] = 60.0f / (float)ratio;
                //         if (ptr == nbSamples)
                //         {
                //             ptr = 0;
                //             float res = 0;
                //             for (auto x : timeCounter)
                //             {
                //                 res += x;
                //             }
                //             std::cout << "Real FPS: " << res / nbSamples << std::endl;
                //         }
                //     }
                }

                mainWindow.Update(true);
            }
        }

        // audioSystem.Shutdown();
    }

    return 0;
}