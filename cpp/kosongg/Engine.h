#pragma once

struct SDL_Window;
typedef void *SDL_GLContext;

namespace kosongg {

class EngineBase {

protected:
  SDL_Window *m_window;
  SDL_GLContext m_glcontext;
  char* m_glsl_version;
  bool m_show_demo_window;
  bool m_show_another_window;
  unsigned int m_clear_color;
  float m_hidpi_x, m_hidpi_y;
  int m_window_width, m_window_height;
  int m_screen_width, m_screen_height;

  void InitSDL();
  void InitImGui();
  virtual void RunImGui();

public:
   EngineBase(/* dependency */);
  ~EngineBase();

  EngineBase(EngineBase &other) = delete;
  void operator=(const EngineBase &) = delete;

  virtual void Init() = 0;
  void Run();
  virtual void Clean() = 0;
};

};