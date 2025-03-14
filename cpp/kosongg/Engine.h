#pragma once

#include <string>
#include <vector>

struct SDL_Window;
typedef void *SDL_GLContext;

#if defined(__APPLE__) && defined(BUILD_APPLE_BUNDLE)
std::string GetBundleResourcePath(const char * path);
#endif

namespace kosongg {

class EngineBase {

protected:
	SDL_Window *  m_sdlWindow;
	SDL_GLContext m_glContext;
	const char *  m_windowTitle;
	const char *  m_glslVersionStr;

	unsigned int  m_clearColor;
	float         m_hidpiX;
	float         m_hidpiY;
	int           m_windowWidth;
	int           m_windowHeight;
	int           m_screenWidth;
	int           m_screenHeight;
	std::string   m_imguiConfigPath;
	int           m_toolBarSize;
	int           m_menuBarHeight;
	unsigned int  m_dockSpaceID;

	void InitSDL();
	void InitImGui();
	virtual void RunImGui() = 0;
	bool BeginDockSpace();
	void EndDockSpace();
	bool BeginToolBar();
	void EndToolBar();
	int GetToolBarSize();

public:
	EngineBase(/* dependency */);
	virtual ~EngineBase();

	EngineBase(EngineBase &other) = delete;
	void operator=(const EngineBase &) = delete;

	virtual void Init(std::vector<std::string> &args);
	void Run();
	virtual void Clean();
	virtual std::string GetResourcePath(const char *path, const char *file);
};

};