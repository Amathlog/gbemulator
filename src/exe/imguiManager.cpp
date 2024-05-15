#include <exe/SDL/mainWindowSDL.h>
#include <exe/imguiManager.h>

#include <core/utils/fileVisitor.h>
#include <cstddef>
#include <exe/imguiWindows/debugWindow.h>
#include <exe/imguiWindows/findRomsWindow.h>
#include <exe/imguiWindows/imguiWindow.h>
#include <exe/imguiWindows/oamWindow.h>
#include <exe/imguiWindows/ramWindow.h>
#include <exe/imguiWindows/tileDataWindow.h>
#include <exe/window.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_sdl2.h>
#include <imgui.h>

#include <ImGuiFileBrowser.h>
#include <core/utils/utils.h>
#include <exe/common.h>
#include <exe/messageService/messageService.h>
#include <exe/messageService/messages/coreMessage.h>
#include <exe/messageService/messages/debugMessage.h>
#include <exe/messageService/messages/screenMessage.h>
#include <exe/messageService/messages/screenPayload.h>
#include <glad/glad.h>
#include <memory>
#include <string>

using GBEmulatorExe::DebugWindow;
using GBEmulatorExe::ImguiManager;

namespace
{
// C++17 fold expression to iterate over all the variadic template parameters
template <typename... Windows>
void CreateAllWindows(ImguiManager::ChildWidgetMap& map)
{
    ([&]() { map.emplace(Windows::GetStaticWindowId(), std::make_unique<Windows>()); }(), ...);
}
} // namespace

ImguiManager::ImguiManager(WindowBase* window)
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    m_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_context);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    if constexpr (ConfigConstants::USE_SDL)
    {
        MainWindowSDL* windowSDL = reinterpret_cast<MainWindowSDL*>(window);
        ImGui_ImplSDL2_InitForOpenGL(windowSDL->GetWindow(), windowSDL->GetGLContext());
    }
    else
    {
        ImGui_ImplGlfw_InitForOpenGL(reinterpret_cast<Window*>(window)->GetWindow(), true);
    }

    ImGui_ImplOpenGL3_Init("#version 330 core");

    m_changeFormats.fill(false);

    m_requestSaveState.fill(false);
    m_requestLoadState.fill(false);

    GetFormatMessage msg;
    if (DispatchMessageServiceSingleton::GetInstance().Pull(msg))
    {
        Format format = msg.GetTypedPayload().m_format;
        if (format != Format::UNDEFINED)
        {
            m_currentFormat = format;
        }

        m_changeFormats[(unsigned)m_currentFormat] = true;
    }

    // Create all windows
    CreateAllWindows<DebugWindow, RamWindow, TileDataWindow, FindRomsWindow, OAMWindow>(m_childWidgets);

    Deserialize();
}

ImguiManager::~ImguiManager()
{
    // Cleanup
    ImGui::SetCurrentContext(m_context);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(m_context);

    Serialize();
}

void DemoWindow()
{
    static bool show_demo_window = true;
    static bool show_another_window = false;
    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    static float f = 0.0f;
    static int counter = 0;

    ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");          // Display some text (you can use a format strings too)
    ImGui::Checkbox("Demo Window", &show_demo_window); // Edit bools storing our window open/close state
    ImGui::Checkbox("Another Window", &show_another_window);

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
    ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

    if (ImGui::Button("Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
        counter++;
    ImGui::SameLine();
    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
    ImGui::End();
}

void ImguiManager::Update()
{
    ImGui::SetCurrentContext(m_context);
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static bool showFPS = false;
    static bool reset = false;

    if (m_showMainMenu)
    {
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("File"))
        {
            ImGui::MenuItem("Open File", nullptr, &m_showFileExplorer);
            ImGui::MenuItem("Find Roms", nullptr, &m_childWidgets[FindRomsWindow::GetStaticWindowId()]->m_open);
            ImGui::Separator();

            if (ImGui::BeginMenu("State states"))
            {
                for (auto i = 0; i < MAX_SAVE_STATES; ++i)
                {
                    std::string label = "State State ";
                    label += std::to_string(i);
                    ImGui::MenuItem(label.c_str(), nullptr, &m_requestSaveState[i]);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Load states"))
            {
                for (auto i = 0; i < MAX_SAVE_STATES; ++i)
                {
                    std::string label = "State State ";
                    label += std::to_string(i);
                    ImGui::MenuItem(label.c_str(), nullptr, &m_requestLoadState[i]);
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();
            ImGui::MenuItem("Reset", nullptr, &reset);
            ImGui::MenuItem("Exit", nullptr, &m_closeRequested);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Options"))
        {
            if (ImGui::BeginMenu("Screen Format"))
            {
                ImGui::MenuItem("Stretch", nullptr, &m_changeFormats[(unsigned)Format::STRETCH]);
                ImGui::MenuItem("Original", nullptr, &m_changeFormats[(unsigned)Format::ORIGINAL]);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Mode"))
            {
                GetModeMessage message;
                DispatchMessageServiceSingleton::GetInstance().Pull(message);
                const CorePayload& payload = message.GetTypedPayload();

                m_modes[0] = false;
                m_modes[1] = false;

                m_modes[(int)payload.m_mode] = true;

                ImGui::BeginDisabled(!payload.m_GBModeEnabled);
                ImGui::MenuItem("GB", nullptr, &m_modes[0]);
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!payload.m_GBCModeEnabled);
                ImGui::MenuItem("GBC", nullptr, &m_modes[1]);
                ImGui::EndDisabled();

                ImGui::EndMenu();
            }

            // ImGui::MenuItem("Enable Audio", nullptr, &m_isSoundEnabled.value);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Debug"))
        {
            ImGui::MenuItem("Show FPS", nullptr, &showFPS);
            ImGui::MenuItem("Ram visualizer", nullptr, &m_childWidgets[RamWindow::GetStaticWindowId()]->m_open);
            ImGui::MenuItem("Disassembly", nullptr, &m_childWidgets[DebugWindow::GetStaticWindowId()]->m_open);
            ImGui::MenuItem("Tile data", nullptr, &m_childWidgets[TileDataWindow::GetStaticWindowId()]->m_open);
            ImGui::MenuItem("OAM", nullptr, &m_childWidgets[OAMWindow::GetStaticWindowId()]->m_open);
            ImGui::MenuItem("Break on start", nullptr, &m_breakOnStart.value);
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    HandleFileExplorer();
    HandlePerf(showFPS);

    for (auto i = 0; i < MAX_SAVE_STATES; ++i)
    {
        if (m_requestSaveState[i])
        {
            DispatchMessageServiceSingleton::GetInstance().Push(SaveStateMessage("", (int)i));
            m_requestSaveState[i] = false;
            break;
        }

        if (m_requestLoadState[i])
        {
            DispatchMessageServiceSingleton::GetInstance().Push(LoadStateMessage("", (int)i));
            m_requestLoadState[i] = false;
            break;
        }
    }

    // Check format
    for (auto i = 0; i < m_changeFormats.size(); ++i)
    {
        if (m_changeFormats[i] && i != (size_t)m_currentFormat)
        {
            if (m_currentFormat != Format::UNDEFINED)
                m_changeFormats[(unsigned)m_currentFormat] = false;

            m_currentFormat = (Format)i;
            DispatchMessageServiceSingleton::GetInstance().Push(ChangeFormatMessage(m_currentFormat));
            break;
        }
    }

    // Check mode
    if (m_modes[0] && m_modes[1])
    {
        GetModeMessage message;
        DispatchMessageServiceSingleton::GetInstance().Pull(message);
        const CorePayload& payload = message.GetTypedPayload();

        ChangeModeMessage changeModeMessage(payload.m_mode == GBEmulator::Mode::GB ? GBEmulator::Mode::GBC
                                                                                   : GBEmulator::Mode::GB);
        DispatchMessageServiceSingleton::GetInstance().Push(changeModeMessage);

        m_modes[(int)payload.m_mode] = false;
    }

    // Check reset
    if (reset)
    {
        DispatchMessageServiceSingleton::GetInstance().Push(ResetMessage());
        reset = false;
    }

    // Check break
    HandleBreakOnStart();

    for (auto& childWidget : m_childWidgets)
    {
        childWidget.second->Draw();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    ImGui::EndFrame();

    // Update and Render additional Platform Windows
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void ImguiManager::HandleBreakOnStart()
{
    if (m_breakOnStart.value != m_breakOnStart.previous)
    {
        m_breakOnStart.previous = m_breakOnStart.value;
        DispatchMessageServiceSingleton::GetInstance().Push(SetBreakOnStartMessage(m_breakOnStart.value));
    }
}

void ImguiManager::HandleFileExplorer()
{
    if (m_showFileExplorer)
        ImGui::OpenPopup("Open File");

    m_showFileExplorer = false;

    static imgui_addons::ImGuiFileBrowser fileDialog;

    if (fileDialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(0, 0),
                                  ".gb,.gbc"))
    {
        // Load a new file
        if (!fileDialog.selected_path.empty())
        {
            DispatchMessageServiceSingleton::GetInstance().Push(LoadNewGameMessage(fileDialog.selected_path));
        }
    }
}

void ImguiManager::HandlePerf(bool showFPS)
{
    if (!showFPS)
        return;

    ImGuiIO& io = ImGui::GetIO();
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                    ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                    ImGuiWindowFlags_NoNav;
    {
        const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = 0.0f;
        window_pos.y = 30.0f;
        window_pos_pivot.x = 0.0f;
        window_pos_pivot.y = 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        window_flags |= ImGuiWindowFlags_NoMove;
    }
    ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
    if (ImGui::Begin("FPS", &showFPS, window_flags))
    {
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                    ImGui::GetIO().Framerate);
        // GetFrametimeMessage message;
        // if(DispatchMessageServiceSingleton::GetInstance().Pull(message))
        // {
        //     const float* frametimes = reinterpret_cast<const float*>(message.GetTypedPayload().m_dataPtr);
        //     size_t size = message.GetTypedPayload().m_dataSize;
        //     float mean = 0.0f;
        //     for (auto i = 0; i < size; ++i)
        //     {
        //         mean += frametimes[i];
        //     }
        //     mean /= (float)size;
        //     float fps = 1000.0f / mean;

        //     ImGui::Text("Game average %.3f ms/frame (%.1f FPS)", mean, fps);
        //     ImGui::PlotLines("33 ms\n\n\n\n\n0ms", frametimes, (int)size, (int)message.GetTypedPayload().m_offset,
        //     nullptr, 0.0f, 33.f, ImVec2(0, 80.0f));
        // }
    }
    ImGui::End();
}

std::string ImguiCachePath() { return (GBEmulator::Utils::GetExePath() / "imgui.cache").string(); }

void ImguiManager::Serialize()
{
    GBEmulator::Utils::FileWriteVisitor visitor(ImguiCachePath(), true);

    // Write window open status
    visitor.WriteValue(m_childWidgets.size());
    for (const auto& widget : m_childWidgets)
    {
        visitor.WriteValue(widget.first);
        visitor.WriteValue(widget.second->m_open);
    }

    visitor.WriteValue(m_isSoundEnabled.value);
    visitor.WriteValue(m_currentFormat);
    visitor.WriteValue(m_breakOnStart.value);

    if (visitor.GetVersion() >= GBEmulator::Utils::FileVersion::Version_0_0_2)
    {
        visitor.WriteContainer(m_modes);
    }
}

void ImguiManager::Deserialize()
{
    GBEmulator::Utils::FileReadVisitor visitor(ImguiCachePath(), true);

    if (!visitor.IsValid() || visitor.GetVersion() != GBEmulator::Utils::FileVersion::CurrentVersion)
    {
        // Do not load if we have a mismatch
        return;
    }

    // Version 0.0.1
    size_t windowStatusSize = 0;
    visitor.ReadValue(windowStatusSize);

    for (size_t i = 0; i < windowStatusSize; ++i)
    {
        ChildWidgetMap::key_type id = 0;
        bool opened = false;
        visitor.ReadValue(id);
        visitor.ReadValue(opened);

        auto it = m_childWidgets.find(id);
        if (it != m_childWidgets.end())
        {
            it->second->m_open = opened;
        }
    }

    visitor.ReadValue(m_isSoundEnabled.value);
    visitor.ReadValue(m_currentFormat);
    visitor.ReadValue(m_breakOnStart.value);

    if (visitor.GetVersion() >= GBEmulator::Utils::FileVersion::Version_0_0_2)
    {
        visitor.ReadContainer(m_modes);
    }
}