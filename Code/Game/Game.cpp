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
bool _Random_Test(EventParam& param);
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
	SetScreenSize(WORLD_WIDTH, WORLD_HEIGHT);
	tmpTexture = g_theRenderer->AcquireTextureFromFile("Data/Images/Test_StbiFlippedAndOpenGL.png");
	tmpSpriteSheetTexture = g_theRenderer->AcquireTextureFromFile("Data/Images/Test_SpriteSheet8x2.png");
	tmpTestSound = g_theAudio->AcquireSound("Data/Audio/TestSound.mp3");
	theFont = g_theRenderer->AcquireBitmapFontFromFile(g_gameConfigs.GetString("consoleFont", "Ecstasy").c_str());
	DevConsole::s_consoleFont = theFont;

	g_theConsole = new DevConsole(g_theRenderer, 48, 132);
	g_Event->SubscribeEventCallback("Test", DevConsole::Command_Test);
	g_theConsole->Startup();

	
	g_Event->SubscribeEventCallback("Random", _Random_Test);

}

void Game::BeginFrame()
{
	g_theRenderer->BeginFrame();
	g_theConsole->BeginFrame();
}

float age = 0;
int glyphs = 0;

void Game::Update(float deltaSeconds)
{
	age += deltaSeconds;
	static float lastAge = age;
	if (age - lastAge > .1f) {
		lastAge = age;
		//g_theConsole->Print(Stringf("The time is %f", age), Rgba(age/10, 0, 0));
		++glyphs;
	}
}

void Game::Render() const
{
	static SpriteSheet testSpriteSheet(tmpSpriteSheetTexture, IntVec2(8, 2));
	static SpriteAnimationDef testSpriteAnimDef(testSpriteSheet, 0, 15, 4.f, SPRITE_ANIMATION_PINGPONG);

	Camera cam(Vec2(0, 0), Vec2(200, 100));
	g_theRenderer->ClearScreen(Rgba(1.f, 0.f, 1.f));
	g_theRenderer->BeginCamera(cam);

	Vertex_PCU vert[] = {
		Vertex_PCU(Vec3(10,10,0),	Rgba(1.f,1.f, 1.f), Vec2(0.f, 0.f)),
		Vertex_PCU(Vec3(10,100,0),	Rgba(1.f,1.f, 1.f), Vec2(0.f, 1.f)),
		Vertex_PCU(Vec3(100,10,0),	Rgba(1.f,1.f, 1.f), Vec2(1.f, 0.f)),
		Vertex_PCU(Vec3(10,100,0),	Rgba(1.f,1.f, 1.f), Vec2(0.f, 1.f)),
		Vertex_PCU(Vec3(100,10,0),	Rgba(1.f,1.f, 1.f), Vec2(1.f, 0.f)),
		Vertex_PCU(Vec3(100,100,0),	Rgba(1.f,1.f, 1.f), Vec2(1.f, 1.f)),
	};

	g_theRenderer->BindTexture(tmpTexture);
	g_theRenderer->DrawVertexArray(6, vert);

	g_theRenderer->BindTexture(testSpriteSheet.GetTexture());
	Vec2 spriteBottomLeft;
	Vec2 spriteTopRight;

	testSpriteAnimDef.GetFrameAtTime(age).GetUVs(spriteBottomLeft, spriteTopRight);
	//testSpriteSheet.GetSpriteDef(5).GetUVs(spriteBottomLeft, spriteTopRight);
	Vec2 spriteBottomRight(spriteTopRight.x, spriteBottomLeft.y);
	Vec2 spriteTopLeft(spriteBottomLeft.x, spriteTopRight.y);

	Vertex_PCU vertsprite[] = {
		Vertex_PCU(Vec3(120,10,0),	Rgba(1.f,1.f, 1.f), spriteBottomLeft),
		Vertex_PCU(Vec3(136,10,0),	Rgba(1.f,1.f, 1.f), spriteBottomRight),
		Vertex_PCU(Vec3(136,42,0),	Rgba(1.f,1.f, 1.f), spriteTopRight),
		Vertex_PCU(Vec3(120,10,0),	Rgba(1.f,1.f, 1.f), spriteBottomLeft),
		Vertex_PCU(Vec3(120,42,0),	Rgba(1.f,1.f, 1.f), spriteTopLeft),
		Vertex_PCU(Vec3(136,42,0),	Rgba(1.f,1.f, 1.f), spriteTopRight),
	};
	g_theRenderer->DrawVertexArray(6, vertsprite);
	g_theRenderer->BindTexture(nullptr);

	std::vector<Vertex_PCU> vertsForSmooth;
	float(*smoothFunctions[10])(float) = {
		SmoothStep3,
		SmoothStep5,
		SmoothStart2,
		SmoothStart3,
		SmoothStart4,
		SmoothStart5,
		SmoothEnd2,
		SmoothEnd3,
		SmoothEnd4,
		SmoothEnd5,
	};

	//g_theConsole->Clear();
	for (int smoothx = 0; smoothx < 10; ++smoothx) {
		float t = (age/4 - floorf(age/4) - 0.5f) * 2.f + 0.5f;
		t = Clamp(t, 0.f, 1.f);
		Vec2 start(140.f, 5.f + (9 - smoothx) * 5.f);
		Vec2 end(195.f, 5.f + (9 - smoothx + 2) * 5.f);
		float pos = smoothFunctions[smoothx](t);
		Vec2 center = Lerp(start, end, pos);
		AddVerticesOfLine2D(vertsForSmooth, start, end, 0.3f, Rgba::BLACK);
		AddVerticesOfDisk2D(vertsForSmooth, center, 1.f, Rgba::BLACK);
		g_theConsole->Print(Stringf("%i = %f, %f", smoothx, center.x, center.y));
	}
	g_theRenderer->DrawVertexArray(vertsForSmooth.size(), vertsForSmooth);

	_RenderDebugInfo(false);
	_RenderDebugInfo(true);
	
	g_theRenderer->EndCamera(cam);
}

void Game::_RenderDebugInfo(bool afterRender) const
{
	static AABB2 textBox(110.f, 60.f, 190.f, 90.f);
	static float t = age;
	//static std::string text("This\nIs a LongText Tester\nfor my engine\n");
	if (afterRender) {
		std::vector<Vertex_PCU> verts;
		Vec2 alienment(cosf(t) * 1.f, sinf(t) * 1.f);
		alienment.x = FloatMap(alienment.x,-1.f,1.f, 0.f, 1.f);
		alienment.y = FloatMap(alienment.y, -1.f, 1.f, 0.f, 1.f);
		t = age;
		std::string vs = Stringf("This\nIs a LongText Tester\nfor my engine\n(%.2f, %.2f)", alienment.x, alienment.y);
		theFont->AddTextInBox(verts, vs, textBox, alienment, fabsf(10 * sin(age)),
			Rgba::WHITE, TEXT_DRAW_SHRINK_TO_FIT, glyphs);
		g_theRenderer->BindTexture(theFont->GetFontTexture());
		g_theRenderer->DrawVertexArray(verts.size(), verts);
		g_theRenderer->BindTexture(nullptr);

		g_theConsole->RenderConsole();
	}
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


bool _Random_Test(EventParam& param)
{
	UNUSED(param);
	int r = g_rng.GetInt(100);
	float rf = g_rng.GetFloatNormed();
	g_theConsole->Print(Stringf("%i %g", r, rf), Rgba::BLUE);
	return true;
}
