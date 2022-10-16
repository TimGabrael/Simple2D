#pragma once
#include "box2d/box2d.h"

struct PhysicsScene
{
	PhysicsScene(float gravity);
	~PhysicsScene();
	b2World world;
};

PhysicsScene* PH_CreatePhysicsScene(float gravity);

void PH_Update(PhysicsScene* physics, float dt);