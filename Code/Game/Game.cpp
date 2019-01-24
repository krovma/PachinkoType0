#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimationDef.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Console/DevConsole.hpp"
#include "Engine/Event/EventSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <math.h>

//////////////////////////////////////////////////////////////////////////
//Delete these globals
Texture *tmpTexture;
Texture *tmpSpriteSheetTexture;
SoundID tmpTestSound;
BitmapFont* theFont;
//////////////////////////////////////////////////////////////////////////

Game::Game()
{
	m_rng = new RNG();
	m_rng->Init();
	m_flagRunning = true;
}

Game::~Game()
{
	Shutdown();
}

void Game::Startup()
{
	
}

void Game::BeginFrame()
{
	g_theRenderer->BeginFrame();
	g_theConsole->BeginFrame();
}


void Game::Update(float deltaSeconds)
{

}

void Game::Render() const
{
	Camera cam(Vec2(0, 0), Vec2(200, 100));
	g_theRenderer->ClearScreen(Rgba(1.f, 0.f, 1.f));
	g_theRenderer->BeginCamera(cam);

	_RenderDebugInfo(false);
	_RenderDebugInfo(true);
	
	g_theRenderer->EndCamera(cam);
}

void Game::_RenderDebugInfo(bool afterRender) const
{
}

void Game::EndFrame()
{
	g_theRenderer->EndFrame();
	g_theConsole->EndFrame();
}

void Game::Shutdown()
{
}

void Game::GetScreenSize(float *outputWidth, float *outputHeight) const
{
	*outputHeight = m_screenHeight;
	*outputWidth = m_screenWidth;
}

void Game::SetScreenSize(float width, float height)
{
	m_screenWidth = width;
	m_screenHeight = height;
}

void Game::DoKeyDown(unsigned char keyCode)
{
	g_theAudio->PlaySound(tmpTestSound);
	if (keyCode == KEY_SLASH) {
		if (g_theConsole->GetConsoleMode() == CONSOLE_OFF) {
			g_theConsole->SetConsoleMode(CONSOLE_PASSIVE);
		} else {
			g_theConsole->SetConsoleMode(CONSOLE_OFF);
		}
	}
	if (keyCode == KEY_F6) {
		g_Event->Trigger("Test", g_gameConfigs);
		g_theConsole->RunCommandString("Test name=F6 run=true");
	}

	if (keyCode == KEY_F1) {
		g_Event->Trigger("Random");
	}
}

void Game::DoKeyRelease(unsigned char keyCode)
{
	UNUSED(keyCode);
}

void Game::ToggleDebugView()
{
	m_flagDebug = !m_flagDebug;
}
