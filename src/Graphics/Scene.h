#pragma once
#include "../Util/Math.h"

struct Entity
{
	virtual ~Entity() = default;
	virtual void Update(float dt) = 0;
};

enum OBJECT_FLAGS
{
	OBJECT_FLAG_VISIBLE = 1,
};


struct SceneObject
{
	class b2Body* body;
	struct Renderable* renderable;
	struct Entity* entity;
	uint32_t flags;
};

struct Scene* SC_CreateScene();
void SC_CleanUpScene(struct Scene* scene);

void SC_RemoveAll(struct Scene* scene);

SceneObject* SC_AddObject(struct Scene* scene, const SceneObject* obj);
void SC_RemoveObject(struct Scene* scene, SceneObject* obj);

SceneObject** SC_GetAllSceneObjects(struct Scene* scene, uint32_t* num);

void SC_Update(struct Scene* scene, float dt);