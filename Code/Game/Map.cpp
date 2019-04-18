#include "Game/Map.hpp"
#include "Game/Game.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Game/Entity.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Physics/OBBCollider2D.hpp"
#include "Engine/Develop/DebugRenderer.hpp"

#include <iostream>
#include <fstream>
#include "Engine/Physics/CapsuleCollider2D.hpp"

void LoadMap(const char* path)
{
	for (auto each : g_theGame->m_entites) {
		g_GamePhysics->DeleteRigidbody2D(each->GetRigidbody());
		delete each;
	}
	g_theGame->m_entites.clear();

	XmlElement* xml;
	std::string filePath("Data/Maps/");
	filePath += path;
	ParseXmlFromFile(xml, filePath.c_str());
	XmlElement* node = xml->FirstChildElement("OBB");
	while(node != nullptr) {
		DebugRenderer::Log(Stringf("OBB"));
		Vec2 pos = ParseXmlAttr(*node, "pos", Vec2(0, 0));
		Vec2 shape = ParseXmlAttr(*node, "shape", Vec2(1, 1));
		float ori = ParseXmlAttr(*node, "ori",0.f);
		bool dynamic = ParseXmlAttr(*node, "dyn", true);

		Vec3 res = ParseXmlAttr(*node, "restrict", Vec3(0, 0, 0));
		float friction = ParseXmlAttr(*node, "friction", g_theGame->m_defaultFriction);
		float mass = ParseXmlAttr(*node, "mass", g_theGame->m_defaultMass);
		float bounce = ParseXmlAttr(*node, "friction", g_theGame->m_defaultBounce);

		NamedStrings info;
		info.Set("rotation", Stringf("%g", ori));
		info.Set("size", Stringf("%g,%g",shape.x, shape.y));

		Entity* createdEntity = new Entity(g_theGame);
		Rigidbody2D* createdRb = g_GamePhysics->NewRigidbody2D(
			COLLIDER_OBB2,
			info,
			createdEntity->GetTransform()
		);
		createdEntity->SetPosition(pos);
		createdRb->SetRestriction(res);
		createdRb->SetMassKg(mass);
		createdRb->SetBounciness(bounce);
		createdRb->SetFriction(friction);
		createdRb->SetSimulationType(dynamic?PHSX_SIM_DYNAMIC:PHSX_SIM_STATIC);
		createdEntity->BindRigidbody(createdRb);
		g_theGame->m_entites.push_back(createdEntity);

		node = node->NextSiblingElement("OBB");
	}
	node = xml->FirstChildElement("Capsule");
	while (node != nullptr) {
		DebugRenderer::Log(Stringf("Capsule"));
		Vec2 pos = ParseXmlAttr(*node, "pos", Vec2(0, 0));
		Vec2 start = ParseXmlAttr(*node, "start", Vec2(0, 0));
		Vec2 end = ParseXmlAttr(*node, "end", Vec2(0, 0));
		float radius = ParseXmlAttr(*node, "radius", 1.f);
		bool dynamic = ParseXmlAttr(*node, "dyn", true);

		Vec3 res = ParseXmlAttr(*node, "restrict", Vec3(0, 0, 0));
		float friction = ParseXmlAttr(*node, "friction", g_theGame->m_defaultFriction);
		float mass = ParseXmlAttr(*node, "mass", g_theGame->m_defaultMass);
		float bounce = ParseXmlAttr(*node, "friction", g_theGame->m_defaultBounce);

		NamedStrings info;
		info.Set("start", Stringf("%g,%g", start.x, start.y));
		info.Set("end", Stringf("%g,%g", end.x, end.y));
		info.Set("radius", Stringf("%g", radius));

		Entity* createdEntity = new Entity(g_theGame);
		Rigidbody2D* createdRb = g_GamePhysics->NewRigidbody2D(
			COLLIDER_CAPSULE2,
			info,
			createdEntity->GetTransform()
		);
		createdEntity->SetPosition(pos);
		createdRb->SetRestriction(res);
		createdRb->SetMassKg(mass);
		createdRb->SetBounciness(bounce);
		createdRb->SetFriction(friction);
		createdRb->SetSimulationType(dynamic ? PHSX_SIM_DYNAMIC : PHSX_SIM_STATIC);
		createdEntity->BindRigidbody(createdRb);
		g_theGame->m_entites.push_back(createdEntity);

		node = node->NextSiblingElement("Capsule");
	}
}

void SaveMap(const char* path)
{
	std::string filePath("Data/Maps/");
	filePath += path;
	std::ofstream fout(filePath);
	fout << "<Map>\n";
	for (auto each : g_theGame->m_entites) {
		auto collider = each->GetRigidbody()->GetCollider();
		auto rb = each->GetRigidbody();
		switch(collider->m_type) {
		case COLLIDER_AABB2: break;
		case COLLIDER_DISK2D: break;
		case COLLIDER_OBB2: {
			OBB2 shape = dynamic_cast<const OBBCollider2D*>(collider)->GetWorldShape();
			Vec2 size = shape.GetSize();
			auto pos = each->GetPosition();
			float orientation = Atan2Degrees(shape.GetRight().y, shape.GetRight().x);
			Vec3 res = rb->GetRestriction();
			fout << "\t<OBB " << "pos=\"" << pos.x << "," << pos.y << "\" shape=\"" << size.x << "," << size.y << "\" ori=\"" << orientation << "\" dyn=" <<
				(rb->GetSimulationType() == PHSX_SIM_DYNAMIC ? "\"true\" " : "\"false\" ")
				<< "restrict=\""<< res.x << "," <<res.y<<","<<res.z<<"\" " << "bounce=\"" << rb->GetBounciness() <<"\" friction=\"" << rb->GetFriction() <<"\" mass=\""
				<< rb->GetMassKg() << "\""
				<< "/>\n";
			break;
		}
		case COLLIDER_CAPSULE2: {
			Capsule2 shape = dynamic_cast<const CapsuleCollider2D*>(collider)->GetWorldShape();
			float radius = shape.Radius;
			auto pos = each->GetPosition();
			Vec3 res = rb->GetRestriction();
			fout << "\t<Capsule " << "pos=\"" << pos.x << "," << pos.y << "\" start=\"" << shape.Start.x << "," << shape.Start.y << "\" end=\"" << shape.End.x << "," << shape.End.y << "\" radius=\"" << radius << "\" dyn=" <<
				(each->GetRigidbody()->GetSimulationType() == PHSX_SIM_DYNAMIC ? "\"true\"" : "\"false\"")
				<< "restrict=\"" << res.x << "," << res.y << "," << res.z << "\" " << "bounce=\"" << rb->GetBounciness() << "\" friction=\"" << rb->GetFriction() << "\" mass=\""
				<< rb->GetMassKg() << "\""
				<< "/>\n";
			break;
		}
		default: ;
		}
	}
	fout << "</Map>" << std::endl;
	fout.close();
}