#include "Engine.h"

#include "../../src/MainApp.h"

int main(int argc, char ** argv)
{
    MainApp *mainApp = MainApp::GetInstance();
    mainApp->Init();
    mainApp->Run();
    mainApp->Clean();

    return 0;
}
