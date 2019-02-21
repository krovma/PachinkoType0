#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Game//Entity.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture2D.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimationDef.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Console/DevConsole.hpp"
#include "Engine/Event/EventSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <math.h>
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/AABBCollider2D.hpp"
#include "Engine/Physics/DiskCollider2D.hpp"

//////////////////////////////////////////////////////////////////////////
//Delete these globals
PhysicsSystem* g_GamePhysics = nullptr;
//////////////////////////////////////////////////////////////////////////
PhysicsSimulationType _possessedSimType;


Game::Game()
{
	m_rng = new RNG();
	m_rng->Init();
	m_flagRunning = true;
	SetScreenSize(137.5f, 100);
}

Game::~Game()
{
	Shutdown();
}

void Game::Startup()
{
	g_Event->SubscribeEventCallback("Test", DevConsole::Command_Test);

	g_GamePhysics = new PhysicsSystem();
	g_GamePhysics->Startup();

	m_cursor = Vec2(137.5f/2.f, 50);
	m_cursorVelocity = Vec2::ZERO;

	m_mainCamera = new Camera(Vec2(0, 0), Vec2(137.5f, 100.f));
	m_shader = g_theRenderer->AcquireShaderFromFile("Data/Shaders/unlit.hlsl");
	m_mainCamera->SetOrthoView(Vec2(0, 0), Vec2(137.5f, 100.f));
	DevConsole::s_consoleFont = g_theRenderer->AcquireBitmapFontFromFile(g_gameConfigs.GetString("consoleFont", "SquirrelFixedFont").c_str());
}

void Game::BeginFrame()
{
	g_theRenderer->BeginFrame();
	g_theConsole->BeginFrame();
	g_GamePhysics->BeginFrame();
}

void Game::Update(float deltaSeconds)
{
	g_theConsole->Update(deltaSeconds);
	m_upSeconds += deltaSeconds;
	m_cursorVelocity.x = Clamp(m_cursorVelocity.x, -50.f, 50.f);
	m_cursorVelocity.y = Clamp(m_cursorVelocity.y, -50.f, 50.f);

	m_cursor += m_cursorVelocity * deltaSeconds;
	m_cursor.x = Clamp(m_cursor.x, 0.f, 137.5f);
	m_cursor.y = Clamp(m_cursor.y, 0.f, 100.f);
	if (m_isSelecting) {
		m_entites[m_possessedEntityIndex]->SetPosition(m_cursor);
	}
	g_GamePhysics->Update(deltaSeconds);
	for (auto eachEntity : m_entites) {
		if (eachEntity->IsOffScreen()) {
			eachEntity->MarkGarbage();
		}
	}
	if (!m_isSelecting) {
		for (auto eachEntityIter = m_entites.begin(); eachEntityIter != m_entites.end();) {
			Entity* eachEntity = *eachEntityIter;
			if (eachEntity->IsGarbage()) {
				g_GamePhysics->DeleteRigidbody2D(eachEntity->GetRigidbody());
				delete eachEntity;
				eachEntityIter = m_entites.erase(eachEntityIter);
			} else {
				++eachEntityIter;
			}
		}
	}
}

void Game::Render() const
{
	RenderTargetView* renderTarget = g_theRenderer->GetFrameColorTarget();
	m_mainCamera->SetRenderTarget(renderTarget);
	g_theRenderer->BeginCamera(*m_mainCamera);
	g_theRenderer->ClearColorTarget(Rgba(0.f, 0.f, 0.f));
	g_theRenderer->BindShader(m_shader);
	_RenderDebugInfo(false);
	//Render cursor here
	std::vector<Vertex_PCU> verts;
	AABB2 cursorBox(m_cursor - Vec2(0.75f, 0.75f), m_cursor + Vec2(0.75f, 0.75f));
	AddVerticesOfLine2D(verts, cursorBox.Min, cursorBox.Max, 0.2f, Rgba::LIME);
	AddVerticesOfLine2D(
		verts,
		Vec2(cursorBox.Min.x, cursorBox.Max.y),
		Vec2(cursorBox.Max.x, cursorBox.Min.y),
		0.2f, Rgba::LIME
	);
	g_theRenderer->BindTextureView(0, nullptr);
	g_theRenderer->DrawVertexArray(verts.size(), verts);
	_RenderDebugInfo(true);
	verts.clear();
	g_theRenderer->BindTextureView(0, DevConsole::s_consoleFont->GetFontTexture());
	std::string info = Stringf("Obj: %u Mass:%s Bounce: %.2f Friction: %.2f", m_entites.size()
		, m_autoMass ? "[A]uto" : Stringf(" %.2f",m_defaultMass).c_str(),
		m_defaultBounce, 1.f - m_defaultSmooth);
	DevConsole::s_consoleFont->AddVertsForText2D(verts, Vec2(0, 98.f), 1.5f, info, Rgba::SILVER);
	if (m_isSelecting) {
		Rigidbody2D* rb = m_entites[m_possessedEntityIndex]->GetRigidbody();
		info = Stringf("Selected>> Mass: %.2f Bounce: %.2f Friction: %.2f"
			, rb->GetMassKg(), rb->GetBounciness(), 1.f - rb->GetSmoothness());
		DevConsole::s_consoleFont->AddVertsForText2D(verts, Vec2(0, 96.f), 1.5f, info, Rgba::LIME);
	}
	g_theRenderer->DrawVertexArray(verts.size(), verts);

	g_theConsole->RenderConsole();
	g_theRenderer->EndCamera(*m_mainCamera);
}

void Game::_RenderDebugInfo(bool afterRender) const
{
	if (afterRender)
		return;
	int size = (int)m_entites.size();
	for (int i = 0; i < size; ++i) {
		Rigidbody2D* rb = m_entites[i]->GetRigidbody();
		const Collider2D* col = rb->GetCollider();
		PhysicsSimulationType simulation = rb->GetSimulationType();
		Rgba color = Rgba::YELLOW;
		if (simulation == PHSX_SIM_STATIC && col->m_inCollision) {
			color = Rgba::MAGENTA;
		}
		if (simulation == PHSX_SIM_DYNAMIC && !col->m_inCollision) {
			color = Rgba::BLUE;
		}
		if (simulation == PHSX_SIM_DYNAMIC && col->m_inCollision) {
			color = Rgba::RED;
		}
		if (m_isSelecting && i == m_possessedEntityIndex) {
			color = Rgba::WHITE;
		}
		rb->GetCollider()->DebugRender(g_theRenderer, color);
	}
}

////////////////////////////////
void Game::_possessNearest()
{
	float minDist2 = 9999999.f;
	int closetEntityIndex = -1;
	int size = (int)m_entites.size();
	for (int i = m_firstPossessable; i < size; ++i) {
		if (m_entites[i]->IsGarbage()) {
			continue;
		}
		Entity* entity = m_entites[i];
		Collider2D* col = (Collider2D*)entity->GetRigidbody()->GetCollider();
		float dist2;
		if (col->m_type == COLLIDER_AABB2) {
			Vec2 closestPoint = GetNearestPointOnAABB2(m_cursor, ((AABBCollider2D*)col)->GetWorldShape());
			dist2 = GetDistanceSquare(m_cursor, closestPoint);
			if (dist2 < minDist2) {
				minDist2 = dist2;
				closetEntityIndex = i;
			}
		} else {
			Vec2 closestPoint = entity->GetPosition();
			closestPoint = m_cursor - closestPoint;
			closestPoint.ClampLength(((DiskCollider2D*)col)->GetRadius());
			closestPoint += entity->GetPosition();
			dist2 = GetDistanceSquare(m_cursor, closestPoint);
			if (dist2 < minDist2) {
				minDist2 = dist2;
				closetEntityIndex = i;
			}
		}
	}
	if (closetEntityIndex >= 0)
		_setPossessInfo(closetEntityIndex);
}

////////////////////////////////
void Game::_setPossessInfo(int index)
{
	m_cursor = m_entites[index]->GetPosition();
	m_possessedEntityIndex = index;
	Rigidbody2D* rb = (Rigidbody2D*)m_entites[m_possessedEntityIndex];
	_possessedSimType = rb->GetSimulationType();
	rb->SetSimulationType(PHSX_SIM_STATIC);
	m_isSelecting = true;
}

////////////////////////////////
void Game::_SpawnBox(const Vec2& position, PhysicsSimulationType simulation)
{
	std::string shape;
	float w = g_rng.GetFloatInRange(FloatRange(2.f, 5.f));
	float h = g_rng.GetFloatInRange(FloatRange(2.f, 5.f));
	shape = Stringf("%g,%g;%g,%g", -w, -h, w, h);
	NamedStrings info;
	info.Set("localShape", shape.c_str());
	Entity* createdEntity = new Entity(this);
	Rigidbody2D* createdRb = g_GamePhysics->NewRigidbody2D(
		COLLIDER_AABB2,
		info,
		createdEntity->GetTransform()
	);
	createdEntity->SetPosition(position);
	if (m_autoMass) {
		createdRb->SetMassKg(w * h * 0.1f);
	} else {
		createdRb->SetMassKg(m_defaultMass);
	}
	createdRb->SetBounciness(m_defaultBounce);
	createdRb->SetSmoothness(m_defaultSmooth);
	createdRb->SetSimulationType(simulation);
	createdEntity->BindRigidbody(createdRb);
	m_entites.push_back(createdEntity);
}

////////////////////////////////
void Game::_SpawnDisk(Vec2& position, PhysicsSimulationType simulation)
{
	float r = g_rng.GetFloatInRange(FloatRange(1.f, 10.f));
	NamedStrings info;
	info.Set("radius", Stringf("%g", r).c_str());
	Entity* createdEntity = new Entity(this);
	Rigidbody2D* createdRb = g_GamePhysics->NewRigidbody2D(
		COLLIDER_DISK2D,
		info,
		createdEntity->GetTransform()
	);
	createdEntity->SetPosition(position);
	if (m_autoMass) {
		createdRb->SetMassKg(3.1415926f * r * r * 0.1f);
	} else {
		createdRb->SetMassKg(m_defaultMass);
	}
	createdRb->SetBounciness(m_defaultBounce);
	createdRb->SetSmoothness(m_defaultSmooth);
	createdRb->SetSimulationType(simulation);
	createdEntity->BindRigidbody(createdRb);
	m_entites.push_back(createdEntity);
}

void Game::EndFrame()
{
	g_theRenderer->EndFrame();
	g_theConsole->EndFrame();
	g_GamePhysics->EndFrame();
}

void Game::Shutdown()
{
	g_GamePhysics->Shutdown();
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
	if (IsConsoleUp()) {
		if (keyCode == KEY_ESC) {
			g_theConsole->KeyPress(CONSOLE_ESC);
		} else if (keyCode == KEY_ENTER) {
			g_theConsole->KeyPress(CONSOLE_ENTER);
		} else if (keyCode == KEY_BACKSPACE) {
			g_theConsole->KeyPress(CONSOLE_BACKSPACE);
		} else if (keyCode == KEY_LEFTARROW) {
			g_theConsole->KeyPress(CONSOLE_LEFT);
		} else if (keyCode == KEY_RIGHTARROW) {
			g_theConsole->KeyPress(CONSOLE_RIGHT);
		} else if (keyCode == KEY_UPARROW) {
			g_theConsole->KeyPress(CONSOLE_UP);
		} else if (keyCode == KEY_DOWNARROW) {
			g_theConsole->KeyPress(CONSOLE_DOWN);
		} else if (keyCode == KEY_DELETE) {
			g_theConsole->KeyPress(CONSOLE_DELETE);
		} else if (keyCode == KEY_F6) {
			g_Event->Trigger("test", g_gameConfigs);
			//g_theConsole->RunCommandString("Test name=F6 run=true");
		} else if (keyCode == KEY_F1) {
			g_Event->Trigger("random");
		}
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	if (keyCode == KEY_SLASH) {
		if (g_theConsole->GetConsoleMode() == CONSOLE_OFF) {
			g_theConsole->SetConsoleMode(CONSOLE_PASSIVE);
		} else {
			g_theConsole->SetConsoleMode(CONSOLE_OFF);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	if (!m_isSelecting) {
		if (keyCode == KEY_A) {
			m_autoMass = !m_autoMass;
		}
		if (keyCode == KEY_F1) {
			_SpawnBox(m_cursor, PHSX_SIM_STATIC);
		}
		if (keyCode == KEY_F2) {
			_SpawnDisk(m_cursor, PHSX_SIM_STATIC);
		}
		if (keyCode == KEY_F3) {
			_SpawnBox(m_cursor, PHSX_SIM_DYNAMIC);
		}
		if (keyCode == KEY_F4) {
			_SpawnDisk(m_cursor, PHSX_SIM_DYNAMIC);
		}
		if (keyCode == KEY_TAB) {
			_possessNearest();
		}
	} else {
		if (keyCode == KEY_TAB) {
			Rigidbody2D* rb = (Rigidbody2D*)m_entites[m_possessedEntityIndex];
			rb->SetSimulationType(_possessedSimType);
			do {
				++m_possessedEntityIndex;
				if (m_possessedEntityIndex >= (int)m_entites.size()) {
					m_possessedEntityIndex = m_firstPossessable;
				}
			} while (m_entites[m_possessedEntityIndex]->IsGarbage());
			_setPossessInfo(m_possessedEntityIndex);
		}
		if (keyCode == KEY_SPACE) {
			m_isSelecting = false;
			Rigidbody2D* rb = (Rigidbody2D*)m_entites[m_possessedEntityIndex];
			rb->SetSimulationType(_possessedSimType);
		}
		if (keyCode == KEY_DELETE) {
			m_entites[m_possessedEntityIndex]->MarkGarbage();
			m_isSelecting = false;
		}
	}

	if (keyCode == KEY_N) {
		if (m_isSelecting) {
			Rigidbody2D* selected = m_entites[m_possessedEntityIndex]->GetRigidbody();
			selected->SetMassKg(Clamp(selected->GetMassKg() - 0.1f, 0.f, 9999999.f));
		} else {
			m_defaultMass = Clamp(m_defaultMass - 0.1f, 0.1f, 9999999.f);
		}
	}
	if (keyCode == KEY_M) {
		if (m_isSelecting) {
			Rigidbody2D* selected = m_entites[m_possessedEntityIndex]->GetRigidbody();
			selected->SetMassKg(Clamp(selected->GetMassKg() + 0.1f, 0.f, 9999999.f));
		} else {
			m_defaultMass = Clamp(m_defaultMass + 0.1f, 0.1f, 9999999.f);
		}
	}
	if (keyCode == KEY_COMMA) {
		if (m_isSelecting) {
			Rigidbody2D* selected = m_entites[m_possessedEntityIndex]->GetRigidbody();
			selected->SetBounciness(Clamp(selected->GetBounciness() - 0.1f, 0.f, 1.f));
		} else {
			m_defaultBounce = Clamp(m_defaultBounce - 0.1f, 0.f, 1.f);
		}
	}
	if (keyCode == KEY_PERIOD) {
		if (m_isSelecting) {
			Rigidbody2D* selected = m_entites[m_possessedEntityIndex]->GetRigidbody();
			selected->SetBounciness(Clamp(selected->GetBounciness() + 0.1f, 0.f, 1.f));
		} else {
			m_defaultBounce = Clamp(m_defaultBounce + 0.1f, 0.f, 1.f);
		}
	}

	if (keyCode == KEY_UPARROW) {
		m_cursorVelocity += Vec2(0.f, 50.f);
	}
	if (keyCode == KEY_DOWNARROW) {
		m_cursorVelocity += Vec2(0.f, -50.f);
	}
	if (keyCode == KEY_LEFTARROW) {
		m_cursorVelocity += Vec2(-50.f, 0.f);
	}
	if (keyCode == KEY_RIGHTARROW) {
		m_cursorVelocity += Vec2(50.f, 0.f);
	}
}

void Game::DoKeyRelease(unsigned char keyCode)
{
	if (IsConsoleUp())
		return;

	if (keyCode == KEY_UPARROW) {
		m_cursorVelocity -= Vec2(0.f, 50.f);
	}
	if (keyCode == KEY_DOWNARROW) {
		m_cursorVelocity -= Vec2(0.f, -50.f);
	}
	if (keyCode == KEY_LEFTARROW) {
		m_cursorVelocity -= Vec2(-50.f, 0.f);
	}
	if (keyCode == KEY_RIGHTARROW) {
		m_cursorVelocity -= Vec2(50.f, 0.f);
	}
}

void Game::ToggleDebugView()
{
	m_flagDebug = !m_flagDebug;
}

////////////////////////////////
void Game::DoChar(char charCode)
{
	if (!IsConsoleUp())
		return;
	g_theConsole->Input(charCode);
}