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
#include "Engine/Develop/DevConsole.hpp"
#include "Engine/Event/EventSystem.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/FloatRange.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Physics/OBBCollider2D.hpp"
#include "Engine/Physics/CapsuleCollider2D.hpp"
#include "Engine/Physics/DiskCollider2D.hpp"
#include "Engine/Develop/DebugRenderer.hpp"
#include "Engine/Core/WindowContext.hpp"
#include <math.h>

ConstantBuffer *_buf;
extern WindowContext* g_theWindow;
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

	m_cursor = Vec2(137.5f / 2.f, 50);
	m_cursorVelocity = Vec2::ZERO;

	m_mainCamera = new Camera(Vec2(0, 0), Vec2(137.5f, 100.f));
	m_mainCamera->SetResolution(Vec2(RESOLUTION_WIDTH, RESOLUTION_HEIGHT));
	m_worldBound = AABB2(-26.f, -20, 163.5f, 120.f);
	m_cameraCenter = Vec2(68.75f, 50.f);
	m_cameraExtend = m_cameraCenter;

	m_mainCamera->SetCameraModel(
		Mat4::MakeTranslate3D(Vec3(m_cameraCenter, 1.f))
	);
	m_mainCamera->SetRenderTarget(g_theRenderer->GetFrameColorTarget());
	m_shader = Shader::CreateShaderFromXml("Data/Shaders/unlit.shader.xml", g_theRenderer);
	m_mainCamera->SetOrthoView(-m_cameraExtend, m_cameraExtend);
	DevConsole::s_consoleFont = g_theRenderer->AcquireBitmapFontFromFile(g_gameConfigs.GetString("consoleFont", "SquirrelFixedFont").c_str());
	_buf = g_theRenderer->GetModelBuffer();
	g_theRenderer->BindConstantBuffer(CONSTANT_SLOT_MODEL, _buf);
	_buf->Buffer(&Mat4::Identity, sizeof(Mat4));

	g_theWindow->LockMouse();
	g_theWindow->HideMouse();

	float clientSpaceToWorldScaler = 137.5f / g_theRenderer->GetResolution().x;
	m_cursor = Vec2(g_theWindow->GetClientMousePosition());
	m_cursor /= 0.97f;
	m_cursor *= clientSpaceToWorldScaler;
	m_cursor.x = Clamp(m_cursor.x, 0.f, 137.5f);
	m_cursor.y = Clamp(m_cursor.y, 0.f, 100.f);
	m_cursor.y = 100.f - m_cursor.y;
}

void Game::BeginFrame()
{
	g_theRenderer->BeginFrame();
	g_theConsole->BeginFrame();
	g_GamePhysics->BeginFrame();
}

void Game::Update(float deltaSeconds)
{
	static float clientSpaceToWorldScaler = 137.5f / g_theRenderer->GetResolution().x;
	static float worldToClientSpaceScaler = g_theRenderer->GetResolution().x / 137.5f;
	g_theConsole->Update(deltaSeconds);
	m_upSeconds += deltaSeconds;
	/*m_cursorVelocity.x = Clamp(m_cursorVelocity.x, -50.f, 50.f);
	m_cursorVelocity.y = Clamp(m_cursorVelocity.y, -50.f, 50.f);

	m_cursor += m_cursorVelocity * deltaSeconds;
	*/
	 {
		m_cursor = Vec2(g_theWindow->GetClientMousePosition());
		m_cursor /= 0.971f;
		m_cursor = m_mainCamera->ClientToWorld(m_cursor);
	}
	DebugRenderer::DrawText2D(
		AABB2(Vec2(500,500), Vec2(600,600)
		),
		DevConsole::s_consoleFont, 20.f, Stringf("%g, %g", m_cursor.x, m_cursor.y), 0.f, BitmapFont::ALIGHMENT_LEFT
	);
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

	if (m_mouseDown) {
		m_cursorPosB = m_cursor;
		Vec2 t1 = m_mainCamera->WorldToClient(m_cursorPosA); t1.y = RESOLUTION_HEIGHT - t1.y;
		Vec2 t2 = m_mainCamera->WorldToClient(m_cursorPosB); t2.y = RESOLUTION_HEIGHT - t2.y;

		DebugRenderer::DrawLine2D(t1,t2, 1.f, 0.f);
	}

	DebugRenderer::DrawLine2D(m_mainCamera->WorldToClient(m_worldBound.GetBottomLeft()),	m_mainCamera->WorldToClient(m_worldBound.GetBottomRight()), 25.f, 0.f);
	DebugRenderer::DrawLine2D(m_mainCamera->WorldToClient(m_worldBound.GetBottomRight()),	m_mainCamera->WorldToClient(m_worldBound.GetTopRight()), 25.f, 0.f);
	DebugRenderer::DrawLine2D(m_mainCamera->WorldToClient(m_worldBound.GetTopRight()),		m_mainCamera->WorldToClient(m_worldBound.GetTopLeft()), 25.f, 0.f);
	DebugRenderer::DrawLine2D(m_mainCamera->WorldToClient(m_worldBound.GetTopLeft()),		m_mainCamera->WorldToClient(m_worldBound.GetBottomLeft()), 25.f, 0.f);


}

void Game::Render() const
{
	static float worldToClientSpaceScaler = g_theRenderer->GetResolution().x / 137.5f;

	RenderTargetView* renderTarget = g_theRenderer->GetFrameColorTarget();
	m_mainCamera->SetRenderTarget(renderTarget);
	g_theRenderer->BeginCamera(*m_mainCamera);
	g_theRenderer->ClearColorTarget(Rgba(0.f, 0.f, 0.f));
	g_theRenderer->BindShader(m_shader);
	_buf = g_theRenderer->GetModelBuffer();
	g_theRenderer->BindConstantBuffer(CONSTANT_SLOT_MODEL, _buf);
	//_buf->Buffer(&Mat4::Identity, sizeof(Mat4));
	_RenderDebugInfo(false);
	//Render cursor here
	//std::vector<Vertex_PCU> verts;
	AABB2 cursorBox(m_cursor - Vec2(0.75f, 0.75f), m_cursor + Vec2(0.75f, 0.75f));
	cursorBox.Min = m_mainCamera->WorldToClient(cursorBox.Min);
	cursorBox.Max = m_mainCamera->WorldToClient(cursorBox.Max);
	cursorBox.Min.y = RESOLUTION_HEIGHT - cursorBox.Min.y;

	cursorBox.Max.y = RESOLUTION_HEIGHT - cursorBox.Max.y;
	DebugRenderer::DrawLine2D(cursorBox.Min, cursorBox.Max, 1.f, 0.f, Rgba::LIME);
	DebugRenderer::DrawLine2D(Vec2(cursorBox.Min.x, cursorBox.Max.y),
		Vec2(cursorBox.Max.x, cursorBox.Min.y), 1.f, 0.f, Rgba::LIME);

	//g_theRenderer->BindTextureViewWithSampler(0, DevConsole::s_consoleFont->GetFontTexture());
	std::string info = Stringf("Obj: %u Mass:%s Bounce: %.2f Friction: %.2f", m_entites.size()
		, m_autoMass ? "[A]uto" : Stringf(" %.2f",m_defaultMass).c_str(),
		m_defaultBounce, 1.f - m_defaultSmooth);
	DebugRenderer::DrawText2D(AABB2(0, 980, 1375, 1000), DevConsole::s_consoleFont, 20.f, info, 0.f, BitmapFont::ALIGHMENT_LEFT, Rgba::WHITE);
	//DevConsole::s_consoleFont->AddVertsForText2D(verts, Vec2(0, 98.f), 1.5f, info, Rgba::SILVER);
	if (m_isSelecting) {
		Rigidbody2D* rb = m_entites[m_possessedEntityIndex]->GetRigidbody();
		info = Stringf("Selected>> Mass: %.2f Bounce: %.2f Friction: %.2f"
			, rb->GetMassKg(), rb->GetBounciness(), 1.f - rb->GetSmoothness());
		//DevConsole::s_consoleFont->AddVertsForText2D(verts, Vec2(0, 96.f), 1.5f, info, Rgba::LIME);
		DebugRenderer::DrawText2D(AABB2(0, 960, 1375, 980), DevConsole::s_consoleFont, 20.f, info, 0.f, BitmapFont::ALIGHMENT_LEFT, Rgba::LIME);
	}
	//g_theRenderer->DrawVertexArray(verts.size(), verts);
	info = Stringf("Type: %s Sim: %s", (m_generateOBB?"Box":"Capsule"), (m_generateStatic?"Static":"Dynamic"));
	DebugRenderer::DrawText2D(AABB2(0, 0, 1375, 20), DevConsole::s_consoleFont, 20.f, info, 0.f, BitmapFont::ALIGHMENT_LEFT, Rgba::CYAN);

	_RenderDebugInfo(true);
	g_theConsole->RenderConsole();
	g_theRenderer->EndCamera(*m_mainCamera);
}

void Game::_RenderDebugInfo(bool afterRender) const
{
	if (afterRender) {
		DebugRenderer::Render(m_mainCamera);
		return;
	}
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
		if (col->m_type == COLLIDER_OBB2) {
			OBBCollider2D* t = (OBBCollider2D*)col;
			Vec2 closestPoint = t->GetWorldShape().GetNearestPoint(m_cursor);
			dist2 = GetDistanceSquare(m_cursor, closestPoint);
			if (dist2 < minDist2) {
				minDist2 = dist2;
				closetEntityIndex = i;
			}
		} else if (col->m_type == COLLIDER_CAPSULE2) {
			Vec2 closestPoint = ((CapsuleCollider2D*)col)->GetWorldShape().GetNearestPoint(m_cursor);
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
	//static float worldToClientSpaceScaler = g_theRenderer->GetResolution().x / 137.5f;

	DebugRenderer::DrawText2D(AABB2(100, 100, 200, 200), DevConsole::s_consoleFont, 25, Stringf("%g, %g", m_cursor.x, m_cursor.y), 2.f, BitmapFont::ALIGHMENT_LEFT);
	m_cursor = m_entites[index]->GetPosition();
	DebugRenderer::DrawText2D(AABB2(100, 150, 200, 250), DevConsole::s_consoleFont, 25, Stringf("%g, %g", m_cursor.x, m_cursor.y), 2.f, BitmapFont::ALIGHMENT_LEFT);

	Vec2 clientPos = m_mainCamera->WorldToClient(m_cursor) * 0.971f;

	//Vec2 back = m_mainCamera->ClientToWorld(clientPos);

	g_theWindow->SetClientMousePosition((int)clientPos.x, (int)clientPos.y);
	m_possessedEntityIndex = index;
	Rigidbody2D* rb = m_entites[m_possessedEntityIndex]->GetRigidbody();
	_possessedSimType = rb->GetSimulationType();
	rb->SetSimulationType(PHSX_SIM_STATIC);
	m_isSelecting = true;
}

////////////////////////////////
void Game::_SpawnBox(const Vec2& position, PhysicsSimulationType simulation)
{
	std::string shape;
	std::string rot;

	float w = g_rng.GetFloatInRange(FloatRange(0.5f, 10.f));
	float h = g_rng.GetFloatInRange(FloatRange(0.5f, 10.f));
	float rotation = g_rng.GetFloatInRange(0.f, 360.f);
	shape = Stringf("%g,%g", w, h);
	rot = Stringf("%g", rotation);
	NamedStrings info;
	info.Set("rotation", rot.c_str());
	info.Set("size", shape.c_str());

	Entity* createdEntity = new Entity(this);
	Rigidbody2D* createdRb = g_GamePhysics->NewRigidbody2D(
		COLLIDER_OBB2,
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
void Game::_SpawnOBB(Vec2& start, Vec2& end, PhysicsSimulationType simulation)
{
	std::string shape;
	std::string rot;

	float w = g_rng.GetFloatInRange(FloatRange(0.5f, 10.f));
	float h = GetDistance(end, start);
	float rotation = (end - start).GetRotatedMinus90Degrees().GetAngleDegrees();
	shape = Stringf("%g,%g", w, h);
	rot = Stringf("%g", rotation);
	NamedStrings info;
	info.Set("rotation", rot.c_str());
	info.Set("size", shape.c_str());

	Entity* createdEntity = new Entity(this);
	Rigidbody2D* createdRb = g_GamePhysics->NewRigidbody2D(
		COLLIDER_OBB2,
		info,
		createdEntity->GetTransform()
	);
	createdEntity->SetPosition((start + end) * 0.5f);
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
	float r = g_rng.GetFloatInRange(FloatRange(0.5f, 5.f));
	NamedStrings info;
	info.Set("radius", Stringf("%g", r).c_str());
	info.Set("start", Stringf("%g,%g", m_cursor.x, m_cursor.y).c_str());
	info.Set("end", Stringf("%g,%g", m_cursor.x, m_cursor.y).c_str());
	Entity* createdEntity = new Entity(this);
	Rigidbody2D* createdRb = g_GamePhysics->NewRigidbody2D(
		COLLIDER_CAPSULE2,
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

////////////////////////////////
void Game::_SpawnCapsule(Vec2& start, Vec2& end, PhysicsSimulationType simulation)
{
	float r = g_rng.GetFloatInRange(FloatRange(0.5f, 5.f));
	NamedStrings info;
	info.Set("radius", Stringf("%g", r).c_str());
	info.Set("start", Stringf("%g,%g", start.x, start.y).c_str());
	info.Set("end", Stringf("%g,%g", end.x, end.y).c_str());
	Entity* createdEntity = new Entity(this);
	Rigidbody2D* createdRb = g_GamePhysics->NewRigidbody2D(
		COLLIDER_CAPSULE2,
		info,
		createdEntity->GetTransform()
	);
	createdEntity->SetPosition((start + end) * 0.5f);
	if (m_autoMass) {
		createdRb->SetMassKg(3.1415926f * r * r * 0.1f + 
			(start - end).GetLength() * 2.f * r
		);
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
void Game::_ClampCamera()
{
	if (m_cameraCenter.x - m_cameraExtend.x < m_worldBound.Min.x) {
		m_cameraCenter.x = m_worldBound.Min.x + m_cameraExtend.x;
	}
	if (m_cameraCenter.y - m_cameraExtend.y < m_worldBound.Min.y) {
		m_cameraCenter.y = m_worldBound.Min.y + m_cameraExtend.y;
	}

	if (m_cameraCenter.x + m_cameraExtend.x > m_worldBound.Max.x) {
		m_cameraCenter.x = m_worldBound.Max.x - m_cameraExtend.x;
	}
	if (m_cameraCenter.y + m_cameraExtend.y > m_worldBound.Max.y) {
		m_cameraCenter.y = m_worldBound.Max.y - m_cameraExtend.y;
	}
	m_mainCamera->SetCameraModel(Mat4::MakeTranslate2D(m_cameraCenter));
}

////////////////////////////////
void Game::_RelocateCamera()
{
	m_cameraExtend = Vec2(68.75f, 50.f) / m_currentScale;
	Vec2 currentCursorClient = m_mainCamera->WorldToClient(m_cursor);
	m_mainCamera->SetOrthoView(
		-m_cameraExtend,
		m_cameraExtend
	);
	Vec2 currentCursorShift = m_mainCamera->ClientToWorld(currentCursorClient);
	Vec2 disp = m_cursor - currentCursorShift;
	m_cameraCenter += disp;
	m_mainCamera->SetCameraModel(
		Mat4::MakeTranslate3D(Vec3(m_cameraCenter, 1.f))
	);
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
	//delete m_shader;
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
		if (keyCode == KEY_F3) {
			m_autoMass = !m_autoMass;
		}
		if (keyCode == KEY_F1) {
			m_generateOBB = !m_generateOBB;
		}
		if (keyCode == KEY_F2) {
			m_generateStatic = !m_generateStatic;
		}

	} else {
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

	if (keyCode == KEY_W) {
		m_cameraCenter += Vec2(0.f, 2.f / m_currentScale);
		//m_cursor += Vec2(0.f, 2.f / m_currentScale);

		_ClampCamera();

		//m_mainCamera->SetCameraModel(Mat4::MakeTranslate3D(Vec3(m_cameraCenter)));
	}
	if (keyCode == KEY_A) {
		m_cameraCenter -= Vec2(2.f / m_currentScale, 0.f);
		//m_cursor -= Vec2(2.f / m_currentScale, 0.f);
		_ClampCamera();
		//m_mainCamera->SetCameraModel(Mat4::MakeTranslate3D(Vec3(m_cameraCenter)));
	}
	if (keyCode == KEY_S) {
		m_cameraCenter -= Vec2(0.f, 2.f / m_currentScale);
		//m_cursor -= Vec2(0.f, 2.f / m_currentScale);
		_ClampCamera();
		//m_mainCamera->SetCameraModel(Mat4::MakeTranslate3D(Vec3(m_cameraCenter)));
	}
	if (keyCode == KEY_D) {
		m_cameraCenter += Vec2(2.f / m_currentScale, 0.f);
		//m_cursor += Vec2(2.f / m_currentScale, 0.f);
		_ClampCamera();
		//m_mainCamera->SetCameraModel(Mat4::MakeTranslate3D(Vec3(m_cameraCenter)));
	}
}

void Game::DoKeyRelease(unsigned char keyCode)
{
	if (IsConsoleUp())
		return;
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

////////////////////////////////
bool Game::DoMouseLeftButtonDown()
{
	m_mouseDown = true;
	m_cursorPosA = m_cursor;
	return true;
}

////////////////////////////////
bool Game::DoMouseLeftButtonUp()
{
	//static float worldToClientSpaceScaler = g_theRenderer->GetResolution().x / 137.5f;
	m_mouseDown = false;
	m_cursorPosB = m_cursor;

	if (m_generateOBB) {
		_SpawnOBB(m_cursorPosA, m_cursorPosB, m_generateStatic ? PHSX_SIM_STATIC : PHSX_SIM_DYNAMIC);
	} else {
		_SpawnCapsule(m_cursorPosA, m_cursorPosB, m_generateStatic ? PHSX_SIM_STATIC : PHSX_SIM_DYNAMIC);
	}
	return true;
}

////////////////////////////////
bool Game::DoMouseRightButtonDown()
{
	_possessNearest();
	return true;
}

////////////////////////////////
bool Game::DoMouseRightButtonUp()
{
	if (!m_isSelecting) {
		return true;
	}
	m_isSelecting = false;
	Rigidbody2D* rb = m_entites[m_possessedEntityIndex]->GetRigidbody();
	rb->SetSimulationType(_possessedSimType);
	return true;
}

////////////////////////////////
bool Game::DoMouseWheelDown()
{
	m_currentScale -= 0.1f;
	m_currentScale = Clamp(m_currentScale, 1.f, 3.f);
	_RelocateCamera();
	_ClampCamera();
	return true;
}

////////////////////////////////
bool Game::DoMouseWheelUp()
{
	m_currentScale += 0.1f;
	m_currentScale = Clamp(m_currentScale, 1.f, 3.f);
	_RelocateCamera();
	_ClampCamera();
	return true;
}
