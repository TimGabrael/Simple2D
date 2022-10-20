#pragma once
#include "box2d/box2d.h"

// THIS WHOLE THING SEEMS USELESS MAYBE I SHOULD JUST THROW IT OUT...
// TO BE CONTINUED...
struct PhysicsScene
{
	PhysicsScene(float gravity);
	~PhysicsScene();
	b2World world;
};

PhysicsScene* PH_CreatePhysicsScene(float gravity);
void PH_CleanUpPhysicsScene(PhysicsScene* ph);

void PH_Update(PhysicsScene* physics, float dt);