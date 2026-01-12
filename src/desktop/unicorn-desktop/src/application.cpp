#define NOMINMAX
#include "core/application.h"
#include "core/window.h"
#include "ui/ui_context.h"
#include "ui/ui_renderer.h"
#include "ui/font_manager.h"
#include "audio/sound_manager.h"
#ifdef HAVE_CURL
#include "background/background_manager.h"
#endif
#ifdef HAVE_FFMPEG
#include "video/video_player.h"
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

namespace Unicorn {

    namespace Input {
        static bool IsKeyPressed(int key) {
            auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
            if (!window) return false;
            return glfwGetKey(window, key) == GLFW_PRESS;
        }
    }

    std::string GetMultilingualFontPath() {
#ifdef _WIN32
        std::vector<std::string> paths = {
            "C:\\Windows\\Fonts\\seguiemj.ttf",
            "C:\\Windows\\Fonts\\msgothic.ttc",
            "C:\\Windows\\Fonts\\msyh.ttc",
            "C:\\Windows\\Fonts\\arial.ttf"
        };
#endif
        for (const auto& path : paths) {
            if (std::ifstream(path).good()) {
                return path;
            }
        }
        return paths.back();
    }

    std::string GetArialFontPath() {
#ifdef _WIN32
        char winDir[MAX_PATH];
        if (GetWindowsDirectoryA(winDir, MAX_PATH)) {
            return std::string(winDir) + "\\Fonts\\arial.ttf";
        }
        return "C:\\Windows\\Fonts\\arial.ttf";
#elif defined(__APPLE__)
        std::vector<std::string> paths = {
            "/Library/Fonts/Arial.ttf",
            "/System/Library/Fonts/Supplemental/Arial.ttf",
            "/System/Library/Fonts/Arial.ttf"
        };
        for (const auto& path : paths) {
            if (std::ifstream(path).good()) {
                return path;
            }
        }
        return "/Library/Fonts/Arial.ttf";
#else
        std::vector<std::string> paths = {
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/truetype/msttcorefonts/arial.ttf",
            "/usr/share/fonts/TTF/arial.ttf",
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
        };
        for (const auto& path : paths) {
            if (std::ifstream(path).good()) {
                return path;
            }
        }
        return "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
#endif
    }

    class UnicornApp : public Application {
    public:
        UnicornApp(const ApplicationConfig& config)
            : Application(config),
            m_SelectedPage(0),
            fpsCounter(0)
#ifdef HAVE_CURL
            , m_ApiRequestState(Background::RequestState::Idle)
            , m_CurrentRequestId(0)
            , m_ApiResponseCode(0)
            , m_ApiResponseTime(0.0)
#endif
        {
#ifdef _WIN32
            SetConsoleOutputCP(CP_UTF8);
#endif
#ifdef HAVE_CURL
            m_ApiEndpoint = "https://jsonplaceholder.typicode.com/posts/1";
            m_ApiMethod = Background::RequestMethod::GET;
#endif
        }

        bool FileExists(const std::string& path) {
            std::ifstream file(path);
            return file.good();
        }

        void OnInit() override {
            std::cout << "==================================" << std::endl;
            std::cout << "Unicorn HRMS Starting..." << std::endl;
            std::cout << "==================================" << std::endl;

            auto& audio = GetAudio();
            audio.LoadSound("click", "./assets/sounds/click.wav");
            audio.LoadSound("hover", "./assets/sounds/hover.wav");
            audio.SetMasterVolume(0.7f);


            auto& renderer = GetUI().GetRenderer();
            auto& fontManager = renderer.GetFontManager();

            UI::FontRenderOptions fontOptions;
            fontOptions.useKerning = true;              // Enable kerning for better spacing
            fontOptions.useHinting = true;              // Enable hinting for sharpness
            fontOptions.useAntialiasing = true;         // Critical for quality
            fontOptions.aaMode = UI::FontRenderOptions::AntialiasMode::LCD;  // ⭐ Best quality
            fontOptions.letterSpacing = 0.0f;           // Default spacing
            fontOptions.lineHeight = 1.2f;              // Better line spacing
            fontOptions.weight = 0.0f;                  // Normal weight (0-2)
            fontOptions.baselineOffset = 0.0f;          // No offset


            std::cout << "[HRMS] Loading fonts with enhanced quality..." << std::endl;

#ifdef _WIN32
            std::vector<std::tuple<std::string, std::string, uint32_t>> fontConfigs = {
                {"segoe", "C:\\Windows\\Fonts\\segoeui.ttf", 18},
                {"tahoma", "C:\\Windows\\Fonts\\tahoma.ttf", 18},
                {"arial", "C:\\Windows\\Fonts\\arial.ttf", 18},
                {"notosans", "C:\\Windows\\Fonts\\NotoSans-Regular.ttf", 18},
                {"notosansar", "C:\\Windows\\Fonts\\NotoSansArabic-Regular.ttf", 18}
            };
#else
            std::vector<std::tuple<std::string, std::string, uint32_t>> fontConfigs = {
                {"dejavu", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", 18},
                {"liberation", "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf", 18}
            };
#endif

            bool fontLoaded = false;
            std::string activeFontName;

            for (const auto& [fontName, fontPath, fontSize] : fontConfigs) {
                std::cout << "[HRMS]   Trying: " << fontPath << " @ " << fontSize << "px" << std::endl;

                FILE* testFile = fopen(fontPath.c_str(), "rb");
                if (!testFile) {
                    std::cout << "[HRMS]     ✗ File not accessible" << std::endl;
                    continue;
                }
                fclose(testFile);
                std::cout << "[HRMS]     ✓ File exists" << std::endl;

                if (fontManager.LoadFontWithOptions(fontName, fontPath, fontSize, fontOptions)) {
                    std::cout << "[HRMS]     ✓ Font loaded successfully" << std::endl;

                    if (!fontLoaded) {
                        fontManager.SetActiveFont(fontName);
                        activeFontName = fontName;
                        fontLoaded = true;
                        std::cout << "[HRMS]     ✓ Set as active font" << std::endl;
                    }
                }
                else {
                    std::cout << "[HRMS]     ✗ Font loading failed" << std::endl;
                }
            }

            if (!fontLoaded) {
                std::cerr << "[HRMS]   ✗✗✗ CRITICAL: No fonts loaded!" << std::endl;
            }
            else {
                std::cout << "[HRMS]   ✓✓✓ Active font: " << activeFontName << std::endl;

                // fontManager.GetRenderOptions().weight = 0.3f;  // Slightly bold
                // fontManager.GetRenderOptions().letterSpacing = 0.5f;  // More spacing
            }

#ifdef HAVE_CURL
            auto& bgManager = Background::BackgroundManager::Get();
            bgManager.Init();
            bgManager.SetMaxConcurrentRequests(5);
            std::cout << "[HRMS] Background API Manager: Enabled" << std::endl;
#else
            std::cout << "[HRMS] Background API Manager: Disabled" << std::endl;
#endif

            std::cout << "==================================" << std::endl;
            std::cout << "✓ Unicorn HRMS Started" << std::endl;
            std::cout << "==================================" << std::endl;
        }

        void OnUpdate(float dt) override {
            static bool escWasPressed = false;
            bool escPressed = Input::IsKeyPressed(GLFW_KEY_ESCAPE);

            if (escPressed && !escWasPressed) {
                Close();
            }
            escWasPressed = escPressed;

#ifdef HAVE_CURL
            auto& bgManager = Background::BackgroundManager::Get();
            bgManager.Update();
#endif

#ifdef HAVE_FFMPEG
            if (m_VideoPlayer) {
                m_VideoPlayer->Update(dt);
            }
#endif
        }

        void OnRender() override {
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }

        void OnUIRender() override {
            auto& ui = GetUI();
            uint32_t windowWidth = GetWindow().GetWidth();
            uint32_t windowHeight = GetWindow().GetHeight();

            ui.SetScrollPhysics(0.88f, 350.0f, 26.0f);

            static int lastSelectedPage = m_SelectedPage;
            bool pageChanged = (m_SelectedPage != lastSelectedPage);

            if (pageChanged) {
                ui.MarkDirty();
                lastSelectedPage = m_SelectedPage;

                // Note: Scroll position is now saved per-page automatically
                // No need to reset - it will restore the previous position
            }

            RenderSidebar(ui, 2.0f, windowHeight);
            RenderMainContent(ui, windowWidth, windowHeight);

            if (m_SelectedPage == 3 && ui.IsDirty()) {
                int newFps = static_cast<int>(1.0f / glm::max(ui.GetDeltaTime(), 0.001f));
                if (abs(newFps - fpsCounter) > 5) {
                    fpsCounter = newFps;
                }
            }
        }

        void OnShutdown() override {
#ifdef HAVE_CURL
            auto& bgManager = Background::BackgroundManager::Get();
            bgManager.Shutdown();
#endif
            std::cout << "[App] Shutting down..." << std::endl;
        }

    private:
        void RenderSidebar(UI::UIContext& ui, float contentX, uint32_t windowHeight) {
            const float sidebarWidth = 48.0f;

            UI::WindowBorderStyle borderStyle;
            borderStyle.enabled = true;
            borderStyle.thickness = 1.0f;
            borderStyle.color = UI::Color::Border;
            borderStyle.top = false;
            borderStyle.right = true;
            borderStyle.bottom = false;
            borderStyle.left = false;

            ui.BeginWindow("##sidebar",
                glm::vec2(contentX, 0),
                glm::vec2(sidebarWidth, windowHeight),
                borderStyle
            );

            const float buttonSize = 32.0f;

            if (ui.IconButton("settings", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                m_SelectedPage = 0;
            }

            if (ui.IconButton("person", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                m_SelectedPage = 1;
            }

            if (ui.IconButton("report", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                m_SelectedPage = 2;
            }

#ifdef HAVE_CURL
            if (ui.IconButton("add", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                m_SelectedPage = 5; // API Manager
            }

            if (ui.IconButton("search", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                m_SelectedPage = 6; // API Testing
            }
#endif

            ui.Separator(1.0f, 26);

            if (ui.IconButton("close", glm::vec2(buttonSize, buttonSize), UI::Alignment::Center)) {
                Close();
            }

            ui.EndWindow();
        }

        void RenderMainContent(UI::UIContext& ui, uint32_t windowWidth, uint32_t windowHeight) {
            const float sidebarWidth = 50.0f;
            const float margin = 20.0f;
            const float contentX = sidebarWidth + margin;
            const float contentWidth = windowWidth - sidebarWidth - (margin * 2);

            ui.BeginGlobalScroll(
                glm::vec2(contentX, 0),
                glm::vec2(contentWidth, windowHeight)
            );

            switch (m_SelectedPage) {
            case 0: RenderSettings(ui, contentX, contentWidth); break;
            case 1: RenderEmployees(ui, contentX, contentWidth); break;
            case 2: RenderReports(ui, contentX, contentWidth); break;
            case 3: RenderCodeTools(ui, contentX, contentWidth); break;
#ifdef HAVE_FFMPEG
            case 4: RenderVideoPlayer(ui, contentX, contentWidth); break;
#endif
#ifdef HAVE_CURL
            case 5: RenderAPIManager(ui, contentX, contentWidth); break;
            case 6: RenderAPITesting(ui, contentX, contentWidth); break;
#endif
            }

            ui.EndGlobalScroll();
        }

        void RenderSettings(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.85f, 700.0f);

            ui.BeginWindow("Settings",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 500)
            );

            ui.TextColored(UI::Color::Primary, "Application Settings");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            static bool notifications = true;
            static bool darkMode = false;
            static float volume = 0.7f;

            ui.Text("General Settings:");
            ui.Spacing();

            ui.Checkbox("Enable Notifications", &notifications);
            ui.Spacing();

            ui.Checkbox("Dark Mode", &darkMode);
            ui.Spacing();

            ui.Text("Volume:");
            ui.SliderFloat("##volume", &volume, 0.0f, 1.0f);

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 250.0f);
            if (ui.ButtonWithIcon("settings", "Save Settings", glm::vec2(buttonWidth, 42))) {
                std::cout << "Settings saved" << std::endl;
            }

            ui.EndWindow();
        }

        void RenderEmployees(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.95f, 900.0f);

            ui.BeginWindow("الموظفين",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 600)
            );

            ui.TextColored(UI::Color::Primary, "إدارة الموظفين");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 250.0f);
            if (ui.ButtonWithIcon("person", "إضافة موظف جديد", glm::vec2(buttonWidth, 42))) {
                std::cout << "Add employee clicked" << std::endl;
            }

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.Text("قائمة الموظفين:");
            ui.Spacing();

            float panelWidth = windowWidth - 40.0f; // Leave margin for window padding

            ui.BeginScrollablePanel("employee_list", glm::vec2(panelWidth, 350), UI::BorderStyle::Outset);
            {

                float itemWidth = panelWidth - 30.0f; // Leave space for scrollbar (12px) + padding

                for (int i = 0; i < 50; i++) {
                    ui.Panel(glm::vec2(itemWidth, 60), [&]() {
                        std::string name = "موظف #" + std::to_string(i + 1);
                        ui.TextColored(UI::Color::Black, name);
                        ui.TextColored(UI::Color::TextSecondary, "الوظيفة: محاسب");
                        });
                    ui.Spacing(5.0f);
                }
            }
            ui.EndScrollablePanel();

            ui.EndWindow();
        }

        void RenderReports(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.9f, 800.0f);

            ui.BeginWindow("Reports",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 550)
            );

            ui.TextColored(UI::Color::Primary, "Reports & Analytics");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 250.0f);
            if (ui.ButtonWithIcon("report", "Generate Report", glm::vec2(buttonWidth, 42))) {
                std::cout << "Generate report clicked" << std::endl;
            }

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.Text("Monthly Performance Report");
            ui.Spacing();

            float panelWidth = windowWidth - 20.0f;
            for (int i = 0; i < 10; i++) {
                ui.Panel(glm::vec2(panelWidth, 50), [&]() {
                    std::string month = "Month " + std::to_string(i + 1);
                    ui.Text(month);
                    ui.Text("Performance: 85%");
                    });
                ui.Spacing(5.0f);
            }

            ui.EndWindow();
        }

        void RenderCodeTools(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.95f, 900.0f);

            ui.BeginWindow("Code Tools",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 650)
            );

            ui.TextColored(UI::Color::Primary, "Developer Tools");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.Text("Debug Information:");
            ui.Spacing();

            std::string mousePos = "Mouse: " +
                std::to_string((int)ui.GetMousePos().x) + ", " +
                std::to_string((int)ui.GetMousePos().y);
            ui.Text(mousePos);

            ui.Spacing();

            ui.Text("Rendering Stats:");
            ui.Text("FPS: " + std::to_string(fpsCounter));

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.Text("Scrollable Content Test:");
            ui.Spacing();

            float panelWidth = windowWidth - 20.0f;
            for (int i = 0; i < 20; i++) {
                ui.Panel(glm::vec2(panelWidth, 50), [&]() {
                    std::string text = "Debug Line #" + std::to_string(i + 1);
                    ui.Text(text);
                    });
                ui.Spacing(5.0f);
            }

            ui.EndWindow();
        }

        void RenderVideoPlayer(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.95f, 1000.0f);

            ui.BeginWindow("Video Player",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 700)
            );

            ui.TextColored(UI::Color::Primary, "Video Player");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            static std::string videoPath = "./test.mp4";
            static bool isURL = false;

            ui.Text("Video Source:");
            ui.Spacing();

            if (ui.InputText("##videopath", videoPath, 512)) {
                GetUI().MarkDirty();
            }

            ui.Checkbox("Load from URL", &isURL);
            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 200.0f);
            if (ui.ButtonWithIcon("settings", "Load Video", glm::vec2(buttonWidth, 42))) {
#ifdef HAVE_FFMPEG
                if (m_VideoPlayer) {
                    m_VideoPlayer->LoadVideo(videoPath, isURL);
                }
#endif
            }

            ui.EndWindow();
        }

#ifdef HAVE_CURL
        void RenderAPIManager(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.95f, 950.0f);

            ui.BeginWindow("API Manager",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 750)
            );

            ui.TextColored(UI::Color::Primary, "HTTP Request Manager");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.Text("API Endpoint:");
            ui.Spacing();

            if (ui.InputText("##api_endpoint", m_ApiEndpoint, 512)) {
                ui.MarkDirty();
            }

            ui.Spacing();

            ui.Text("Method:");
            ui.Spacing();

            ui.BeginHorizontal();

            if (ui.Button("GET", glm::vec2(70, 35))) {
                m_ApiMethod = Background::RequestMethod::GET;
            }

            if (ui.Button("POST", glm::vec2(70, 35))) {
                m_ApiMethod = Background::RequestMethod::POST;
            }

            if (ui.Button("PUT", glm::vec2(70, 35))) {
                m_ApiMethod = Background::RequestMethod::PUT;
            }

            if (ui.Button("DELETE", glm::vec2(70, 35))) {
                m_ApiMethod = Background::RequestMethod::HTTP_DELETE;
            }

            ui.EndHorizontal();

            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 220.0f);

            bool isLoading = (m_ApiRequestState == Background::RequestState::Loading);

            if (!isLoading) {
                if (ui.Button("Execute Request", glm::vec2(buttonWidth, 44))) {
                    ExecuteAPIRequest();
                }
            }
            else {
                if (ui.Button("Cancel", glm::vec2(buttonWidth, 44))) {
                    CancelAPIRequest();
                }
            }

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.TextColored(UI::Color::Primary, "Status");
            ui.Spacing();

            std::string stateText = "State: ";
            glm::vec4 stateColor = UI::Color::Text;

            switch (m_ApiRequestState) {
            case Background::RequestState::Idle:
                stateText += "Ready";
                stateColor = UI::Color::TextSecondary;
                break;
            case Background::RequestState::Loading:
                stateText += "Loading...";
                stateColor = UI::Color::Primary;
                break;
            case Background::RequestState::Success:
                stateText += "Success";
                stateColor = glm::vec4(0.2f, 0.8f, 0.2f, 1.0f);
                break;
            case Background::RequestState::Error:
                stateText += "Error";
                stateColor = glm::vec4(0.9f, 0.2f, 0.2f, 1.0f);
                break;
            case Background::RequestState::Cancelled:
                stateText += "Cancelled";
                stateColor = UI::Color::TextDisabled;
                break;
            }

            ui.TextColored(stateColor, stateText);
            ui.Spacing();

            if (m_ApiResponseCode > 0) {
                std::string codeText = "HTTP Status: " + std::to_string(m_ApiResponseCode);
                ui.Text(codeText);

                std::string timeText = "Response Time: " +
                    std::to_string(static_cast<int>(m_ApiResponseTime * 1000)) + " ms";
                ui.Text(timeText);
                ui.Spacing();
            }

            auto& bgManager = Background::BackgroundManager::Get();
            std::string activeText = "Active: " +
                std::to_string(bgManager.GetActiveRequestCount());
            ui.Text(activeText);

            std::string pendingText = "Queued: " +
                std::to_string(bgManager.GetPendingRequestCount());
            ui.Text(pendingText);

            if (!m_ApiError.empty()) {
                ui.Spacing();
                ui.TextColored(glm::vec4(0.9f, 0.2f, 0.2f, 1.0f),
                    "Error: " + m_ApiError);
            }

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.TextColored(UI::Color::Primary, "Response");
            ui.Spacing();

            if (!m_ApiResponseBody.empty()) {
                float panelWidth = windowWidth - 20.0f;
                ui.BeginScrollablePanel("api_response", glm::vec2(panelWidth, 300),
                    UI::BorderStyle::Inset);
                {
                    std::istringstream stream(m_ApiResponseBody);
                    std::string line;

                    while (std::getline(stream, line)) {
                        if (!line.empty()) {
                            ui.TextColored(UI::Color::Black, line);
                            ui.Spacing(2.0f);
                        }
                    }
                }
                ui.EndScrollablePanel();
            }

            ui.EndWindow();
        }

        void ExecuteAPIRequest() {
            Background::RequestOptions options;
            options.url = m_ApiEndpoint;
            options.method = m_ApiMethod;
            options.timeoutSeconds = 30;

            options.headers["Content-Type"] = "application/json";
            options.headers["Accept"] = "application/json";

            auto& bgManager = Background::BackgroundManager::Get();

            m_CurrentRequestId = bgManager.Request(
                options,
                [this](Background::RequestState state, const Background::Response& response) {
                    OnAPIRequestComplete(state, response);
                }
            );

            m_ApiRequestState = Background::RequestState::Loading;
            m_ApiResponseBody.clear();
            m_ApiError.clear();
            m_ApiResponseCode = 0;

            GetUI().MarkDirty();
        }

        void CancelAPIRequest() {
            if (m_CurrentRequestId > 0) {
                auto& bgManager = Background::BackgroundManager::Get();
                bgManager.Cancel(m_CurrentRequestId);

                m_ApiRequestState = Background::RequestState::Cancelled;
                GetUI().MarkDirty();
            }
        }

        void OnAPIRequestComplete(Background::RequestState state,
            const Background::Response& response) {
            m_ApiRequestState = state;
            m_ApiResponseBody = response.body;
            m_ApiError = response.error;
            m_ApiResponseCode = response.statusCode;
            m_ApiResponseTime = response.elapsedTime;

            GetUI().MarkDirty();

            if (state == Background::RequestState::Success) {
                std::cout << "[API] Request completed in "
                    << static_cast<int>(response.elapsedTime * 1000) << "ms" << std::endl;
            }
            else if (state == Background::RequestState::Error) {
                std::cerr << "[API] Request failed: " << response.error << std::endl;
            }
        }

        void RenderAPITesting(UI::UIContext& ui, float contentX, float contentWidth) {
            float windowWidth = glm::min(contentWidth * 0.95f, 1000.0f);

            ui.BeginWindow("API Testing Suite",
                glm::vec2(contentX, 30),
                glm::vec2(windowWidth, 800)
            );

            ui.TextColored(UI::Color::Primary, "HTTP Request Testing & Diagnostics");
            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            auto& bgManager = Background::BackgroundManager::Get();
            auto stats = bgManager.GetStats();

            ui.TextColored(UI::Color::Primary, "Statistics");
            ui.Spacing();

            ui.Text("Total Requests: " + std::to_string(stats.totalRequests));
            ui.Text("Successful: " + std::to_string(stats.successfulRequests));
            ui.Text("Failed: " + std::to_string(stats.failedRequests));
            ui.Text("Cancelled: " + std::to_string(stats.cancelledRequests));

            if (stats.totalRequests > 0) {
                std::string avgTime = "Avg Response Time: " +
                    std::to_string(static_cast<int>(stats.averageResponseTime * 1000)) + " ms";
                ui.Text(avgTime);
            }

            std::string downloaded = "Downloaded: " +
                std::to_string(stats.totalBytesDownloaded / 1024) + " KB";
            ui.Text(downloaded);

            std::string uploaded = "Uploaded: " +
                std::to_string(stats.totalBytesUploaded / 1024) + " KB";
            ui.Text(uploaded);

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.TextColored(UI::Color::Primary, "Quick Tests");
            ui.Spacing();

            float buttonWidth = glm::min(windowWidth - 40.0f, 250.0f);

            if (ui.Button("Test GET Request", glm::vec2(buttonWidth, 40))) {
                TestGETRequest();
            }

            if (ui.Button("Test POST Request", glm::vec2(buttonWidth, 40))) {
                TestPOSTRequest();
            }

            if (ui.Button("Test Multiple Concurrent", glm::vec2(buttonWidth, 40))) {
                TestConcurrentRequests();
            }

            if (ui.Button("Test Connection", glm::vec2(buttonWidth, 40))) {
                TestConnection();
            }

            if (ui.Button("Test Timeout", glm::vec2(buttonWidth, 40))) {
                TestTimeout();
            }

            if (ui.Button("Test Large Download", glm::vec2(buttonWidth, 40))) {
                TestLargeDownload();
            }

            ui.Spacing();
            ui.Separator(1.0f, windowWidth - 40.0f);
            ui.Spacing();

            ui.TextColored(UI::Color::Primary, "Request History");
            ui.Spacing();

            auto history = bgManager.GetRequestHistory(20);

            if (!history.empty()) {
                float panelWidth = windowWidth - 20.0f;
                ui.BeginScrollablePanel("request_history", glm::vec2(panelWidth, 300),
                    UI::BorderStyle::Inset);
                {
                    for (const auto& entry : history) {
                        ui.Panel(glm::vec2(panelWidth - 20.0f, 50), [&]() {
                            ui.TextColored(UI::Color::Black, entry);
                            });
                        ui.Spacing(3.0f);
                    }
                }
                ui.EndScrollablePanel();
            }
            else {
                ui.TextColored(UI::Color::TextSecondary, "No requests yet");
            }

            ui.Spacing();

            if (ui.Button("Clear History", glm::vec2(buttonWidth, 40))) {
                bgManager.GetRequestHistory(0);
            }

            ui.EndWindow();
        }

        void TestGETRequest() {
            Background::RequestOptions options;
            options.url = "https://jsonplaceholder.typicode.com/posts/1";
            options.method = Background::RequestMethod::GET;

            auto& bgManager = Background::BackgroundManager::Get();
            bgManager.Request(options, [](Background::RequestState state, const Background::Response& response) {
                if (state == Background::RequestState::Success) {
                    std::cout << "[Test] GET Success: " << response.statusCode << std::endl;
                }
                else {
                    std::cout << "[Test] GET Failed: " << response.error << std::endl;
                }
                });

            std::cout << "[Test] GET request queued" << std::endl;
        }

        void TestPOSTRequest() {
            Background::RequestOptions options;
            options.url = "https://jsonplaceholder.typicode.com/posts";
            options.method = Background::RequestMethod::POST;
            options.body = R"({
                "title": "Test Post",
                "body": "This is a test",
                "userId": 1
            })";
            options.headers["Content-Type"] = "application/json";

            auto& bgManager = Background::BackgroundManager::Get();
            bgManager.Request(options, [](Background::RequestState state, const Background::Response& response) {
                if (state == Background::RequestState::Success) {
                    std::cout << "[Test] POST Success: " << response.statusCode << std::endl;
                }
                else {
                    std::cout << "[Test] POST Failed: " << response.error << std::endl;
                }
                });

            std::cout << "[Test] POST request queued" << std::endl;
        }

        void TestConcurrentRequests() {
            auto& bgManager = Background::BackgroundManager::Get();

            for (int i = 1; i <= 5; i++) {
                Background::RequestOptions options;
                options.url = "https://jsonplaceholder.typicode.com/posts/" + std::to_string(i);
                options.method = Background::RequestMethod::GET;

                bgManager.Request(options, [i](Background::RequestState state, const Background::Response& response) {
                    if (state == Background::RequestState::Success) {
                        std::cout << "[Test] Concurrent #" << i << " Success" << std::endl;
                    }
                    });
            }

            std::cout << "[Test] 5 concurrent requests queued" << std::endl;
        }

        void TestConnection() {
            auto& bgManager = Background::BackgroundManager::Get();

            bool connected = bgManager.TestConnection("https://jsonplaceholder.typicode.com", 5);

            if (connected) {
                std::cout << "[Test] Connection test: SUCCESS" << std::endl;
            }
            else {
                std::cout << "[Test] Connection test: FAILED" << std::endl;
            }
        }

        void TestTimeout() {
            Background::RequestOptions options;
            options.url = "https://httpbin.org/delay/10";
            options.method = Background::RequestMethod::GET;
            options.timeoutSeconds = 3;

            auto& bgManager = Background::BackgroundManager::Get();
            bgManager.Request(options, [](Background::RequestState state, const Background::Response& response) {
                if (state == Background::RequestState::Timeout) {
                    std::cout << "[Test] Timeout test: SUCCESS (timed out as expected)" << std::endl;
                }
                else if (state == Background::RequestState::Success) {
                    std::cout << "[Test] Timeout test: FAILED (completed unexpectedly)" << std::endl;
                }
                else {
                    std::cout << "[Test] Timeout test: ERROR - " << response.error << std::endl;
                }
                });

            std::cout << "[Test] Timeout request queued (3s timeout)" << std::endl;
        }

        void TestLargeDownload() {
            Background::RequestOptions options;
            options.url = "https://jsonplaceholder.typicode.com/posts";
            options.method = Background::RequestMethod::GET;

            auto& bgManager = Background::BackgroundManager::Get();
            bgManager.RequestWithProgress(
                options,
                [](Background::RequestState state, const Background::Response& response) {
                    if (state == Background::RequestState::Success) {
                        std::cout << "[Test] Large download: " << response.downloadSize << " bytes" << std::endl;
                    }
                },
                [](size_t current, size_t total) {
                    if (total > 0) {
                        int percent = static_cast<int>((current * 100) / total);
                        std::cout << "[Test] Progress: " << percent << "%" << std::endl;
                    }
                }
            );

            std::cout << "[Test] Large download queued" << std::endl;
        }
#endif

    private:
#ifdef HAVE_FFMPEG
        std::unique_ptr<Video::VideoPlayer> m_VideoPlayer;
#endif
#ifdef HAVE_CURL
        Background::RequestState m_ApiRequestState;
        Background::RequestMethod m_ApiMethod;
        size_t m_CurrentRequestId;
        std::string m_ApiEndpoint;
        std::string m_ApiResponseBody;
        std::string m_ApiError;
        int m_ApiResponseCode;
        double m_ApiResponseTime;
#endif
        int m_SelectedPage;
        int fpsCounter;
    };

    Application* CreateApplication() {
        ApplicationConfig config;
        config.name = "Unicorn";
        config.width = 1280;
        config.height = 720;
        config.vsync = true;

        return new UnicornApp(config);
    }

} // namespace Unicorn