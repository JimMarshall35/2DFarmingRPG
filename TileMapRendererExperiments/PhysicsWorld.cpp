#include "PhysicsWorld.h"
#include "ECS.h"
#include <memory>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include "flecs.h"
#include "StaticPhysicsBody.h"
#include "QuadTree.h"
#include "Tags.h"
#include <iostream>

PhysicsWorld::PhysicsWorld(b2Vec2 gravity, ECS* ecs, DynamicQuadTreeContainer<flecs::entity>* entityQuadTree)
	:m_b2World(gravity),
	m_timeStep(1.0f / 60.0f),
	m_velocityIterations(6),
	m_positionIterations(2),
	m_ecs(ecs),
	m_entityQuadTree(entityQuadTree)
{
	
}

PhysicsWorld::PhysicsWorld(ECS* ecs, DynamicQuadTreeContainer<flecs::entity>* quadTree)
	:PhysicsWorld(b2Vec2{0,0}, ecs, quadTree)
{

}


DI_FactoryDecl(PhysicsWorld)(ECS* ecs, DynamicQuadTreeContainer<flecs::entity>* quadTree)
{
	DI_FactoryImpl(PhysicsWorld, ecs, quadTree);
}

void PhysicsWorld::Step()
{
	m_b2World.Step(m_timeStep, m_velocityIterations, m_positionIterations);
}

Rect b2AABBToRect(const b2AABB& aabb) {
	Rect rect = { {aabb.lowerBound.x, aabb.lowerBound.y }, { aabb.upperBound.x - aabb.lowerBound.x, aabb.upperBound.y - aabb.lowerBound.y } };
	return rect;
}


bool PhysicsWorld::AddStaticPolygon(const glm::vec2* points, u32 numPoints, flecs::entity& entity)
{
	// convert from glm to b2Vec2
	auto converted = std::make_unique<b2Vec2[]>(numPoints);
	for (int i = 0; i < numPoints; i++) {
		converted[i].Set(points[i].x, points[i].y);
	}

	b2PolygonShape polyShape;
	if(!(3 <= numPoints && numPoints <= b2_maxPolygonVertices)) {
		std::cerr << "polyShape.Set assert would have failed, due to number of points, which is: " << numPoints << "\n";
		return false;
	}
	polyShape.Set(converted.get(), numPoints);
	//p
	b2AABB aabb = { {9999.0f,9999.0f}, {-9999.0f, -9999.0f}, };
	//polyShape.ComputeAABB(&aabb, b2Transform(polyShape.m_centroid, b2Rot(0)), 0);
	for (int i = 0; i < numPoints; i++) {
		if (points[i].x < aabb.lowerBound.x) {
			aabb.lowerBound.x = points[i].x;
		}
		else if (points[i].x > aabb.upperBound.x) {
			aabb.upperBound.x = points[i].x;
		}

		if (points[i].y < aabb.lowerBound.y){
			aabb.lowerBound.y = points[i].y;
		}
		else if (points[i].y > aabb.upperBound.y) {
			aabb.upperBound.y = points[i].y;
		}
	}

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.position = polyShape.m_centroid;
	b2Body* body = m_b2World.CreateBody(&bodyDef);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 1.0f;
	//fixtureDef.filter
	b2Fixture* fixture = body->CreateFixture(&fixtureDef);


	// add to entity
	entity.set<StaticPhysicsBody>({ body, fixture });//.set<StaticPhysicsBody>(body);
	if (!entity.has<IsInQuadTree>()) {
		// add entity to quadtree
		m_entityQuadTree->insert(entity, b2AABBToRect(aabb));
		entity.add<IsInQuadTree>();
	}
	

	return true;
}

bool PhysicsWorld::AddDynamicCircle(const glm::vec2& center, f32 radius, flecs::entity& entity)
{
	b2CircleShape circleShape;
	
	circleShape.m_radius = radius;

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position.Set(center.x,center.y);
	b2Body* body = m_b2World.CreateBody(&bodyDef);
	body->CreateFixture(&circleShape, 1.0f);
	

	return false;
}
