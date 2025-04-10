#include <glad/gl.h>
#ifdef GLAD_GL
    #define IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_opengl3.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <cstdio>
#include <SDL.h>
#include <filesystem>

#include "kosongg/engine.hpp"
#include "kosongg/glutil.hpp"
#include "kosongg/iconsfontawesome6.h"

namespace kosongg {

EngineBase::EngineBase(/* dependency */) {
  m_windowTitle = "My SDL Empty Window";
  m_clearColor = IM_COL32(255,255,255,255); // ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
}

EngineBase::~EngineBase() {}

void EngineBase::InitSDL() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0) {
    printf("Error: %s\n", SDL_GetError());
    return;
  }
// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  m_glslVersionStr = "#version 100";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
  // GL 3.2 Core + GLSL 150
  m_glslVersionStr = "#version 150";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
  // OpenGL 3.3
  m_glslVersionStr = "#version 130";
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

// From 2.0.18: Enable native IME.
#ifdef SDL_HINT_IME_SHOW_UI
  SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif
  SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
  m_windowWidth = 1280;
  m_windowHeight = 720;
  m_sdlWindow = SDL_CreateWindow(m_windowTitle,
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_windowWidth, m_windowHeight, window_flags);

  if (m_sdlWindow == nullptr) {
    printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
  }

  m_glContext = SDL_GL_CreateContext(m_sdlWindow);
  if (m_glContext == nullptr) {
    printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
  }
  SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);
  SDL_GL_SetSwapInterval(1); // Enable vsync
  SDL_GL_GetDrawableSize(m_sdlWindow, &m_screenWidth, &m_screenHeight);
  m_hidpiX = (float)m_screenWidth / m_windowWidth;
  m_hidpiY = (float)m_screenHeight / m_windowHeight;

  int version = gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
  SDL_Log("GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));

#ifdef _WIN32
  //glEnable(GL_DEBUG_OUTPUT);
  //glDebugMessageCallback(OnGLMessageCallback, nullptr);
#endif
}

void EngineBase::InitImGui() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;
  m_imguiConfigPath = GetResourcePath("configs", "imgui.ini");
  io.IniFilename = m_imguiConfigPath.c_str();

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
  //io.ConfigViewportsNoAutoMerge = true;
  //io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Dear ImGui style
  //ImGui::StyleColorsDark();
  ImGui::StyleColorsLight();

  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
  ImGuiStyle& style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
  {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }

  // Setup Platform/Renderer backends
  ImGui_ImplSDL2_InitForOpenGL(m_sdlWindow, m_glContext);
  ImGui_ImplOpenGL3_Init(m_glslVersionStr);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
  //ImFont* font = io.Fonts->AddFontDefault();
  float baseFontSize = 18.0f;
  float iconFontSize = baseFontSize * 4.0f / 5.0f; // FontAwesome fonts need to have their sizes reduced by 2.0f/3.0f in order to align correctly

  ImFontConfig config;
  //config.OversampleH = 2;
  //config.OversampleV = 2;
  config.RasterizerDensity = m_hidpiX;    // 2.0f for retina-display
  //config.GlyphExtraSpacing.x = -0.5f;
  std::string selawikPath = GetResourcePath("kosongg/fonts", "selawkl.ttf");
  ImFont* font = io.Fonts->AddFontFromFileTTF(selawikPath.c_str(), 18.0f, &config);
  if (!font) printf("ERROR: unable to load: %s\n", selawikPath.c_str());

  //ImFont* font = io.Fonts->AddFontFromFileTTF("fonts/SF-Pro-Text-Regular.otf", 14.0f, &config);

  //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
  IM_ASSERT(font != nullptr);

  // merge in icons from Font Awesome
  static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
  ImFontConfig icons_config;
  icons_config.MergeMode = true;
  icons_config.PixelSnapH = true;
  icons_config.RasterizerDensity = m_hidpiX;    // 2.0f for retina-display
  icons_config.GlyphMinAdvanceX = iconFontSize;

  std::string faPath = GetResourcePath("kosongg/fonts", FONT_ICON_FILE_NAME_FAS);
  ImFont* fa_font = io.Fonts->AddFontFromFileTTF(faPath.c_str(), iconFontSize, &icons_config, icons_ranges );
  if (!fa_font) printf("ERROR: unable to load: %s\n", faPath.c_str());

  // use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid
  IM_ASSERT(fa_font != nullptr);

  m_toolBarSize = -1;
}

void EngineBase::Run() {
  SDL_GL_MakeCurrent(m_sdlWindow, m_glContext);

  ImGuiIO& io = ImGui::GetIO(); (void)io;
  ImColor clear_color(m_clearColor);

  // Main loop
  bool done = false;
  while (!done)
  {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
          done = true;
      if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_sdlWindow))
          done = true;
    }
    if (SDL_GetWindowFlags(m_sdlWindow) & SDL_WINDOW_MINIMIZED)
    {
      SDL_Delay(10);
      continue;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    RunImGui();

    // Rendering
    ImGui::Render();

    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(clear_color.Value.x * clear_color.Value.w,
      clear_color.Value.y * clear_color.Value.w,
      clear_color.Value.z * clear_color.Value.w,
      clear_color.Value.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    // For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
      SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }

    SDL_GL_SwapWindow(m_sdlWindow);

    // if (power_saving_mode) SDL_WaitEvent(nullptr);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DeleteContext(m_glContext);
  SDL_DestroyWindow(m_sdlWindow);
  SDL_Quit();
}

void EngineBase::Init(std::vector<std::string> &args) {
  (void)args;
  InitSDL();
  InitImGui();
}

void EngineBase::Clean() {
}

std::string EngineBase::GetResourcePath(const char *path, const char *file) {
  std::filesystem::path spath(path);
  std::filesystem::path sfile(file);
  std::string res((spath / sfile).u8string());
  return res;
}

int EngineBase::GetToolBarSize() {
  if (m_toolBarSize == -1) {
    ImGuiStyle &style = ImGui::GetStyle();
    m_toolBarSize = ImGui::GetFrameHeight() + 2 * style.WindowPadding.y;
    printf("%d\n", m_toolBarSize);
  }
  return m_toolBarSize;
}

bool EngineBase::BeginDockSpace() {
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(viewport->Pos + ImVec2(0, GetToolBarSize()));
  ImGui::SetNextWindowSize(viewport->Size - ImVec2(0, GetToolBarSize()));
  ImGui::SetNextWindowViewport(viewport->ID);
  ImGuiWindowFlags window_flags = 0
          | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking
          | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse
          | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
          | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

  bool ret = ImGui::Begin("MasterDockSpace", NULL, window_flags);
  // Save off menu bar height for later.
  m_menuBarHeight = ImGui::GetCurrentWindow()->MenuBarHeight;

  m_dockSpaceID = ImGui::GetID("MainDockspace");
  ImGui::DockSpace(m_dockSpaceID);

  //ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
  //ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

  return ret;
}

void EngineBase::EndDockSpace () {
  //ImGui::DockBuilderFinish(m_dockSpaceID);

  ImGui::End();
  ImGui::PopStyleVar(3);
}

bool EngineBase::BeginToolBar() {
  ImGuiViewport* viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + m_menuBarHeight));
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, GetToolBarSize()));
  ImGui::SetNextWindowViewport(viewport->ID);

  ImGuiWindowFlags window_flags = 0
          | ImGuiWindowFlags_NoDocking
          | ImGuiWindowFlags_NoTitleBar
          | ImGuiWindowFlags_NoResize
          | ImGuiWindowFlags_NoMove
          | ImGuiWindowFlags_NoScrollbar
          | ImGuiWindowFlags_NoSavedSettings
          ;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
  bool ret = ImGui::Begin("TOOLBAR", NULL, window_flags);
  ImGui::PopStyleVar();

  return ret;
}

void EngineBase::EndToolBar()
{
  ImGui::End();
}

}
