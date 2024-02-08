#pragma once

struct SDL_Window;
struct SDL_Renderer;

namespace kosongg {

class Engine {

protected:
    Engine(/* dependency */);
    ~Engine();

private:
  SDL_Window *m_window;
  SDL_Renderer* m_renderer;

public:
  Engine(Engine &other) = delete;
  void operator=(const Engine &) = delete;

  static Engine *GetInstance(/* dependency */);
  void Init();
  void Run();

};

};