#include "Engine.h"

int main(int argc, char ** argv)
{
    kosongg::Engine *engine = kosongg::Engine::GetInstance();
    engine->Init();
    engine->Run();

    return 0;
}
