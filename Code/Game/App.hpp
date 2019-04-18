#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Game/Game.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Core/WindowContext.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Core/Clock.hpp"

//////////////////////////////////////////////////////////////////////////
class App
{
public:
	App();
	~App();

	void Startup();
	void Shutdown();
	void RunFrame();

	bool IsQuitting() { return m_flagQuit; }
	bool HandleKeyPressed(unsigned char keyCode);
	bool HandleKeyReleased(unsigned char keyCode);
	bool HandleMouseLeftButtonDown();
	bool HandleMouseLeftButtonUp();
	bool HandleMouseRightButtonDown();
	bool HandleMouseRightButtonUp();
	bool HandleMouseWheel(int delta);


	bool HandleQuitRequested();
	bool HandleChar(char charCode);

private:

	Game* m_theGame = nullptr;

	int m_frameCount = 0;
	bool m_flagQuit = false;
	bool m_flagPaused = false;
	bool m_flagSlow = false;

	Clock* m_gameClock = nullptr;
	Clock* m_fixedClock = nullptr;

public:
	Clock* GetGameClock() const
	{
		return m_gameClock;
	}

	Clock* GetFixedClock() const
	{
		return m_fixedClock;
	}
};

extern App* g_theApp;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern WindowContext* g_theWindow;
