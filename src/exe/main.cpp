#include "core/cartridge.h"
#include <exe/mainWindow.h>
#include <core/bus.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <algorithm>
#include <core/utils/fileVisitor.h>


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

    GBEmulator::Utils::FileReadVisitor fileVisitor(path.string());  
    auto cart = std::make_shared<GBEmulator::Cartridge>(fileVisitor);
    bus.InsertCartridge(cart);

    auto previous_point = std::chrono::high_resolution_clock::now();
    constexpr bool showRealFPS = false;
    constexpr size_t nbSamples = 120;
    //std::array<float, nbSamples> timeCounter;
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
                
                mainWindow.Update(true);
            }
        }
    }

    return 0;
}