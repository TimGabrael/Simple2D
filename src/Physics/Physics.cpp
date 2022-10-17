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

	return res;
}
void PH_CleanUpPhysicsScene(PhysicsScene* ph)
{
	delete ph;
}
void PH_Update(PhysicsScene* physics, float dt)
{
	physics->world.Step(dt, 6, 2);
}