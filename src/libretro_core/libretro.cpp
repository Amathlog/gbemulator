#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <libretro.h>

#include <core/bus.h>
#include <core/cartridge.h>
#include <core/constants.h>
#include <core/controller.h>
#include <core/utils/memoryVisitor.h>

#include <array>
#include <cstring>
#include <memory>

static std::unique_ptr<GBEmulator::Bus> s_bus;
static std::shared_ptr<GBEmulator::Controller> s_controller;

static std::array<uint32_t, GBEmulator::GB_INTERNAL_HEIGHT * GBEmulator::GB_INTERNAL_WIDTH> frame_buf;
static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static bool use_audio_cb;

char retro_base_directory[4096];
char retro_game_path[4096];

static retro_environment_t environ_cb;

void retro_init(void)
{
    retro_log_callback log;
    unsigned int level = 4;

    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
    {
        log_cb = log.log;
    }
    else
    {
        log_cb = nullptr;
    }

    environ_cb(RETRO_ENVIRONMENT_SET_PERFORMANCE_LEVEL, &level);

    s_bus = std::make_unique<GBEmulator::Bus>();
    s_controller = std::make_shared<GBEmulator::Controller>();
    s_bus->ConnectController(s_controller);
}

void retro_deinit(void) {}

unsigned retro_api_version(void) { return RETRO_API_VERSION; }

void retro_set_controller_port_device(unsigned port, unsigned device) {}

void retro_get_system_info(struct retro_system_info* info)
{
    std::memset(info, 0, sizeof(*info));
    info->library_name = "gb-emulator";
    info->library_version = "0.1";
    info->need_fullpath = false;
    info->valid_extensions = "gb|gbc";
}

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

void retro_get_system_av_info(struct retro_system_av_info* info)
{
    info->geometry.base_width = GBEmulator::GB_INTERNAL_WIDTH;
    info->geometry.base_height = GBEmulator::GB_INTERNAL_HEIGHT;
    info->geometry.max_width = GBEmulator::GB_INTERNAL_WIDTH;
    info->geometry.max_height = GBEmulator::GB_INTERNAL_HEIGHT;
    info->geometry.aspect_ratio = 0.0f; // 0 = width/height
    info->timing.fps = 60;
    info->timing.sample_rate = GBEmulator::APU_SAMPLE_RATE;
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    if (cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;

    static const struct retro_controller_description controllers[] = {
        {"GB", RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)},
    };

    static const struct retro_controller_info ports[] = {
        {controllers, 1},
        {NULL, 0},
    };

    cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
}

void retro_set_audio_sample(retro_audio_sample_t cb) { audio_cb = cb; }

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb) { audio_batch_cb = cb; }

void retro_set_input_poll(retro_input_poll_t cb) { input_poll_cb = cb; }

void retro_set_input_state(retro_input_state_t cb) { input_state_cb = cb; }

void retro_set_video_refresh(retro_video_refresh_t cb) { video_cb = cb; }

static unsigned phase;

void retro_reset(void) {}

static void update_input(void)
{
    input_poll_cb();

    s_controller->ToggleUp(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP) != 0);
    s_controller->ToggleDown(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN) != 0);
    s_controller->ToggleLeft(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT) != 0);
    s_controller->ToggleRight(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT) != 0);
    s_controller->ToggleA(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A) != 0);
    s_controller->ToggleB(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B) != 0);
    s_controller->ToggleStart(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START) != 0);
    s_controller->ToggleSelect(input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT) != 0);
}

static void check_variables(void) {}

static void video_callback()
{
    std::vector<uint8_t> screen = s_bus->GetPPU().GetScreen();

    for (size_t i = 0; i < frame_buf.size(); ++i)
    {
        const uint32_t R = screen[3 * i];
        const uint32_t G = screen[3 * i + 1];
        const uint32_t B = screen[3 * i + 2];
        frame_buf[i] = (R << 16) | (G << 8) | B;
    }

    video_cb(frame_buf.data(), GBEmulator::GB_INTERNAL_WIDTH, GBEmulator::GB_INTERNAL_HEIGHT,
             GBEmulator::GB_INTERNAL_WIDTH * sizeof(uint32_t));
}

static void audio_callback()
{
    static std::array<float, 128> audio_buffer_float;
    static std::array<int16_t, 128> audio_buffer_int16;

    if (s_bus->GetAPU().FillSamplesIfReady(audio_buffer_float.data()))
    {
        std::transform(audio_buffer_float.begin(), audio_buffer_float.end(), audio_buffer_int16.begin(),
                       [](float x) -> int16_t { return static_cast<int16_t>(std::clamp(x, -1.0f, 1.0f) * 0x7fff); });

        audio_batch_cb(audio_buffer_int16.data(), 64);
    }
}

void retro_run(void)
{
    update_input();

    while (s_bus->GetPPU().GetLY() != 0)
    {
        s_bus->Clock();
        audio_callback();
    }

    while (s_bus->GetPPU().GetLY() <= 143)
    {
        s_bus->Clock();
        audio_callback();
    }

    video_callback();
}

bool retro_load_game(const struct retro_game_info* info)
{
    // struct retro_input_descriptor desc[] = {
    //     {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_LEFT, "Left"},
    //     {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP, "Up"},
    //     {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_DOWN, "Down"},
    //     {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_RIGHT, "Right"},
    //     {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_A, "A"},
    //     {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_B, "B"},
    //     {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_START, "Start"},
    //     {0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_SELECT, "Select"},
    //     {0},
    // };

    // environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc);

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
        return false;
    }

    if (info && info->data)
    {
        GBEmulator::Utils::MemoryReadVisitor visitor(static_cast<const uint8_t*>(info->data), info->size);
        auto Cartridge = std::make_shared<GBEmulator::Cartridge>(visitor);
        s_bus->InsertCartridge(Cartridge);
    }

    // struct retro_audio_callback audio_cb = {audio_callback, audio_set_state};
    // use_audio_cb = environ_cb(RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK, &audio_cb);

    check_variables();

    return true;
}

void retro_unload_game(void) {}

unsigned retro_get_region(void) { return RETRO_REGION_NTSC; }

bool retro_load_game_special(unsigned type, const struct retro_game_info* info, size_t num) { return false; }

size_t retro_serialize_size(void) { return 0; }

bool retro_serialize(void* data_, size_t size) { return false; }

bool retro_unserialize(const void* data_, size_t size) { return false; }

void* retro_get_memory_data(unsigned id)
{
    (void)id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    (void)id;
    return 0;
}

void retro_cheat_reset(void) {}

void retro_cheat_set(unsigned index, bool enabled, const char* code)
{
    (void)index;
    (void)enabled;
    (void)code;
}
