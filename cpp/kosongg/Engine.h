#pragma once

struct SDL_Window;
typedef void *SDL_GLContext;

namespace kosongg {

class EngineBase {

protected:
  SDL_Window *sdlWindow_;
  SDL_GLContext glContext_;
  const char* windowTitle_;
  const char* glslVersionStr_;
  bool showDemoWindow_;
  bool showAnotherWindow_;
  unsigned int clearColor_;
  float hidpiX_, hidpiY_;
  int windowWidth_, windowHeight_;
  int screenWidth_, screenHeight_;

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
};

};