#pragma once

#include <string>

struct SDL_Window;
typedef void *SDL_GLContext;

namespace kosongg {

class EngineBase {

protected:
  SDL_Window *  m_sdlWindow;
  SDL_GLContext m_glContext;
  const char *  m_windowTitle;
  const char *  m_glslVersionStr;
  bool          m_showDemoWindow;
  bool          m_showAnotherWindow;
  unsigned int  m_clearColor;
  float         m_hidpiX;
  float         m_hidpiY;
  int           m_windowWidth;
  int           m_windowHeight;
  int           m_screenWidth;
  int           m_screenHeight;

  void InitSDL();
  void InitImGui();
  virtual void RunImGui();

public:
  EngineBase(/* dependency */);
  ~EngineBase();

  EngineBase(EngineBase &other) = delete;
  void operator=(const EngineBase &) = delete;

  virtual void Init();
  void Run();
  virtual void Clean();
  virtual std::string GetResourcePath(const char *path, const char *file);
};

};