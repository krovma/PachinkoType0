#include "Game/Entity.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include "Engine/Physics/Rigidbody2D.hpp"
#include "Engine/Math/MathUtils.hpp"
//////////////////////////////////////////////////////////////////////////
Entity::Entity(Game *theGame)
	:m_theGame(theGame)
{
}

void Entity::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Entity::Render() const
{
	//GUARANTEE_OR_DIE(_vert != nullptr, "NOTHING TO RENDER\nin <Entity::render>");
	//_game->getRenderer()->DrawVertexArray(_numVert, _vert);
}

bool Entity::IsOffScreen() const
{
	float screenW, screenH;
	m_theGame->GetScreenSize(&screenW, &screenH);
	
	return
		FloatGt(m_transform.Position.x, m_radiusCosmetic + screenW)
		|| FloatGt(m_transform.Position.y, m_radiusCosmetic + screenH)
		|| FloatLt(m_transform.Position.x, -m_radiusCosmetic)
		|| FloatLt(m_transform.Position.y, -m_radiusCosmetic);//not exactly

}

void Entity::MarkGarbage()
{
	m_flagGarbage = true;
	m_flagDead = true;
}

////////////////////////////////
void Entity::BindRigidbody(Rigidbody2D* rigidbody)
{
	m_rigidbody = rigidbody;
	m_rigidbody->UpdateFromTransform();
}

void Entity::SetPosition(const Vec2 &position)
{
	m_transform.Position = position;
}
