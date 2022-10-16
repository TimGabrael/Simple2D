#include "Physics.h"


PhysicsScene::PhysicsScene(float gravity) : world({0.0f, gravity})
{

}
PhysicsScene::~PhysicsScene()
{

}

PhysicsScene* PH_CreatePhysicsScene(float gravity)
{
	PhysicsScene* res = new PhysicsScene(gravity);

	b2BodyDef body;
	body.type = b2_staticBody;
	body.position = { 0.0f, 0.0f };
	body.angle = 0.0f;

	auto b = res->world.CreateBody(&body);


	return res;
}

void PH_Update(PhysicsScene* physics, float dt)
{
	physics->world.Step(dt, 10, 10);
}