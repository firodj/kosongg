#pragma once

struct SDL_Window;
struct SDL_Renderer;

namespace kosongg {

class EngineBase {

protected:
  SDL_Window *m_window;
  SDL_Renderer* m_renderer;



  void InitSDL();
  void InitImGui();

public:
   EngineBase(/* dependency */);
  ~EngineBase();

  EngineBase(EngineBase &other) = delete;
  void operator=(const EngineBase &) = delete;

  virtual void Init() = 0;
  void Run();
};

};