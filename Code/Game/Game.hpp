#pragma once
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/RNG.hpp"
#include "Engine/Physics/PhysicsSystem.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Develop/DevConsole.hpp"

extern RenderContext* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern PhysicsSystem* g_GamePhysics;
extern DevConsole* g_theConsole;

class Camera;
class Shader;
class Entity;

class Game
{
public:
	friend void LoadMap(const char* path);
	friend void SaveMap(const char* path);

	Game();
	~Game();

	bool IsRunning() const { return m_flagRunning; }
	bool IsConsoleUp() const { return (g_theConsole->GetConsoleMode() == CONSOLE_PASSIVE); }
	void Startup();
	void BeginFrame();
	void Update(float deltaSeconds);
	void Render() const;
	void EndFrame();
	void Shutdown();

	void GetScreenSize(float *outputWidth, float *outputHeight) const;
	void SetScreenSize(float width, float height);

	const RenderContext* getRenderer() const { return g_theRenderer; }
	const InputSystem* getInputSystem() const { return g_theInput; }
	const AudioSystem* getAudioSystem() const { return g_theAudio; }
	RNG* getRNG() { return m_rng; }

	//IO
	void DoKeyDown(unsigned char keyCode);
	void DoKeyRelease(unsigned char keyCode);
	void DoChar(char charCode);
	bool DoMouseLeftButtonDown();
	bool DoMouseLeftButtonUp();
	bool DoMouseRightButtonDown();
	bool DoMouseRightButtonUp();
	bool DoMouseWheelDown();
	bool DoMouseWheelUp();

	bool OnGoCollide(EventParam& param);
	bool OnGoEnterTg(EventParam& param);
	bool OnGoLeaveTg(EventParam& param);
	bool OnGoDestroy(EventParam& param);

//DEBUG
	void ToggleDebugView();

private:
	void _RenderDebugInfo(bool afterRender) const;
	void _possessNearest();
	void _setPossessInfo(int index);
	void _SpawnBox(const Vec2& position, PhysicsSimulationType simulation);
	void _SpawnOBB(Vec2& start, Vec2& end, PhysicsSimulationType simulation);
	void _SpawnDisk(Vec2& position, PhysicsSimulationType simulation);
	void _SpawnCapsule(Vec2& start, Vec2& end, PhysicsSimulationType simulation);
	void _ClampCamera();
	Rigidbody2D* _GetNearestRigidBodyAt(const Vec2& screen);
	void _RelocateCamera();
private:
	bool m_flagRunning = false;
	float m_screenWidth;
	float m_screenHeight;
	Camera* m_mainCamera;
	Shader* m_shader;
	RNG* m_rng = nullptr;

	Vec2 m_cursor;

	Vec2 m_cursorPosA;
	Vec2 m_cursorPosB;
	bool m_mouseDown = false;

	Vec2 m_cursorVelocity;
	bool m_autoMass = true;
	float m_defaultMass = 1.f;
	float m_defaultBounce = 1.f;
	float m_defaultFriction = 1.f;
	float m_defaultLinearDrag = 0.f;
	float m_defaultAngularDrag = 0.f;
	int m_possessedEntityIndex;
	bool m_isSelecting = false;
	const int m_firstPossessable = 0;
	bool m_generateOBB = false;
	bool m_generateStatic = false;

	AABB2 m_worldBound;
	Vec2 m_cameraCenter;
	Vec2 m_cameraExtend;
	float m_currentScale = 1.f;


	std::vector<Entity*> m_entites;
//DEBUG
	bool m_createTrigger = false;
	bool m_flagDebug = true;
	float m_upSeconds = 0.f;
};

extern Game* g_theGame;