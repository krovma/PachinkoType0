#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/Transform2D.hpp"
//

class Game;
class Rigidbody2D;

enum EntityTypes
{
	TypeDefault = -1,
	TypeOne = 0,

	NumEntityTypes
};

class Entity
{
public:
	Entity(Game* theGame);

	virtual void Update(float deltaSeconds);
	virtual void Render() const;

	bool IsDead() const { return m_flagDead; }
	bool IsGarbage() const { return m_flagGarbage; }
	bool IsOffScreen() const;
	Vec2 GetPosition() const { return m_transform.Position; }
	float GetRadiusPhysics() const { return m_radiusPhysics; }
	float GetRadiusCosmetic() const { return m_radiusCosmetic; }
	Transform2D* GetTransform() { return &m_transform; }
	
	void MarkGarbage();

	void BindRigidbody(Rigidbody2D* rigidbody);
	Rigidbody2D* GetRigidbody() const { return m_rigidbody; }
	void SetPosition(const Vec2 &position);


protected:
	Game* m_theGame = nullptr;

	Transform2D m_transform;
	Rigidbody2D* m_rigidbody = nullptr;

	float m_radiusPhysics = 0.f;
	float m_radiusCosmetic = 5.f;

	bool m_flagDead = false;
	bool m_flagGarbage = false;
};