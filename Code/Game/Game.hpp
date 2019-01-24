#pragma once
#include "Engine/Renderer/RenderContext.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/RNG.hpp"

#include "Game/GameCommon.hpp"

extern RenderContext* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;

class Game
{
public:
	Game();
	~Game();
	
	bool IsRunning() const { return m_flagRunning; }
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

//DEBUG
	void ToggleDebugView();

private:
	void _RenderDebugInfo(bool afterRender) const;

private:
	bool m_flagRunning = false;
	float m_screenWidth;
	float m_screenHeight;

	RNG* m_rng = nullptr;

//DEBUG
	bool m_flagDebug = false;
	float m_upSeconds = 0.f;
};