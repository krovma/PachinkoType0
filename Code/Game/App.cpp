#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>			// #include this (massive, platform-specific) header in very few places

#include "Game/App.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Develop/DevConsole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Develop/DebugRenderer.hpp"
//////////////////////////////////////////////////////////////////////////
App* g_theApp = nullptr;
InputSystem* g_theInput = nullptr;
RenderContext* g_theRenderer = nullptr;
AudioSystem* g_theAudio = nullptr;
DevConsole* g_theConsole = nullptr;
WindowContext* g_theWindow;
//extern HWND g_hWnd;

//////////////////////////////////////////////////////////////////////////
bool AppQuit_Callback(EventParam& param)
{
	UNUSED(param);
	g_theApp->HandleQuitRequested();
	return true;
}
//////////////////////////////////////////////////////////////////////////
App::App()
{
}
//////////////////////////////////////////////////////////////////////////
App::~App()
{
	Shutdown();
}
//////////////////////////////////////////////////////////////////////////
void App::Startup()
{
	g_theInput = new InputSystem();
	g_theRenderer = new RenderContext(g_theWindow->m_hWnd, RESOLUTION_WIDTH, RESOLUTION_HEIGHT);
	g_theAudio = new AudioSystem();
	g_theInput->StartUp();
	g_theRenderer->Startup();
	g_theConsole = new DevConsole(g_theRenderer, 60, 76);
	g_theConsole->Startup();
	
	DebugRenderer::Startup(g_theRenderer);

	m_theGame = new Game();
	m_theGame->Startup();

	g_Event->SubscribeEventCallback("quit", AppQuit_Callback);
}
//////////////////////////////////////////////////////////////////////////
void App::Shutdown()
{
	m_flagQuit = true;
	if (m_theGame) {
		delete m_theGame;
		m_theGame = nullptr;
	}
	DebugRenderer::Shutdown();
	if (g_theConsole) {
		delete g_theConsole;
		m_theGame = nullptr;
	}
	if (g_theRenderer) {
		g_theRenderer->Shutdown();
		delete g_theRenderer;
		g_theRenderer = nullptr;
	}
	if (g_theInput) {
		g_theInput->Shutdown();
		delete g_theInput;
		g_theInput = nullptr;
	}
	if (g_theAudio) {
		delete g_theAudio;
		g_theAudio = nullptr;
	}
}
//////////////////////////////////////////////////////////////////////////
void App::RunFrame()
{
	static double lastFrameTime = GetCurrentTimeSeconds();
	g_theWindow->BeginFrame();
	g_theInput->BeginFrame();
	g_theAudio->BeginFrame();

	if (!m_theGame->IsRunning()) {
		m_flagQuit = true;
		return;
	}
	m_theGame->BeginFrame();

	static double currentTime;
	currentTime = GetCurrentTimeSeconds();
	double dt = currentTime - lastFrameTime;
	if (m_flagPaused) {
		dt = 0.0;
	} else if (m_flagSlow) {
		dt /= 10.0;
	}

	DebugRenderer::Update((float)dt);
	m_theGame->Update(float(dt));
	m_theGame->Render();

	m_theGame->EndFrame();
	g_theAudio->EndFrame();
	g_theInput->EndFrame();
	g_theWindow->EndFrame();

	++m_frameCount;
	lastFrameTime = currentTime;
}
//////////////////////////////////////////////////////////////////////////
bool App::HandleKeyPressed(unsigned char keyCode)
{
	if (m_theGame->IsConsoleUp()) {
		m_theGame->DoKeyDown(keyCode);
	} else {
		if ('T' == keyCode) {
			m_flagSlow = true;
		} else if ('P' == keyCode) {
			m_flagPaused = !m_flagPaused;
		} else if (0x77 /*F8*/ == keyCode) {
			delete m_theGame;
			m_theGame = new Game();
			m_theGame->Startup();
		} else {
			m_theGame->DoKeyDown(keyCode);
		}
	}
	return true;
}
//////////////////////////////////////////////////////////////////////////
bool App::HandleKeyReleased(unsigned char keyCode)
{
	if (keyCode == 'T') {
		m_flagSlow = false;
	} else {
		m_theGame->DoKeyRelease(keyCode);
	}
	return true;
}

////////////////////////////////
bool App::HandleMouseLeftButtonDown()
{
	m_theGame->DoMouseLeftButtonDown();
	return true;
}

////////////////////////////////
bool App::HandleMouseLeftButtonUp()
{
	m_theGame->DoMouseLeftButtonUp();
	return true;
}

////////////////////////////////
bool App::HandleMouseRightButtonDown()
{
	m_theGame->DoMouseRightButtonDown();
	return true;
}

////////////////////////////////
bool App::HandleMouseRightButtonUp()
{
	m_theGame->DoMouseRightButtonUp();
	return true;
}

////////////////////////////////
bool App::HandleMouseWheel(int delta)
{
	if (delta > 0) {
		m_theGame->DoMouseWheelUp();
	} else {
		m_theGame->DoMouseWheelDown();
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool App::HandleQuitRequested()
{
	m_flagQuit = true;
	return true;
}
////////////////////////////////
bool App::HandleChar(char charCode)
{
	if (m_theGame->IsConsoleUp()) {
		m_theGame->DoChar(charCode);
	}
	return true;
}
