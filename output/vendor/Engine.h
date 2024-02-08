#pragma once

struct SDL_Window;

namespace kosongg {

class Engine {

protected:
    Engine(/* dependency */);
    ~Engine();

private:
  SDL_Window *m_screen;

public:
  Engine(Engine &other) = delete;
  void operator=(const Engine &) = delete;

  static Engine *GetInstance(/* dependency */);
  void Init();
  void Run();

};

};