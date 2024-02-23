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