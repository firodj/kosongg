#include <thread>
#include <SDL.h>
#include "Engine.h"

namespace kosongg {

static Engine* g_engine{nullptr};
static std::mutex g_engine_mutex;

Engine *Engine::GetInstance(/* dependency */)
{
    std::lock_guard<std::mutex> lock(g_engine_mutex);
    if (g_engine == nullptr) {
        g_engine = new Engine(/* dependency */);
    }
    return g_engine;
}

Engine::Engine(/* dependency */) {}

Engine::~Engine() {}

void Engine::Init() {
    SDL_Init(SDL_INIT_VIDEO);

    m_screen = SDL_CreateWindow("My SDL Empty Window",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
}

void Engine::Run() {
    bool quit = false;
    SDL_Event event;

    while (!quit)
    {
        SDL_WaitEvent(&event);

        switch (event.type)
        {
            case SDL_QUIT:
                quit = true;
                break;
        }
    }

    SDL_Quit();
}

}