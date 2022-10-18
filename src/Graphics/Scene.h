#pragma once
#include "../Util/Math.h"

struct Entity
{
	Entity();
	virtual ~Entity();
	virtual void Update(float dt) = 0;
	virtual void UpdateFrame(float dt) = 0;
};

struct Scene* SC_CreateScene();
void SC_CleanUpScene(struct Scene* scene);

void SC_RemoveAll(struct Scene* scene);

void SC_AddEntity(struct Scene* scene, Entity* ent);
void SC_RemoveEntity(struct Scene* scene, Entity* ent);

void SC_AddRenderable(struct Scene* scene, struct Renderable* r);
void SC_RemoveRenderable(struct Scene* scene, struct Renderable* r);


Entity** SC_GetAllEntitys(struct Scene* scene, size_t* num);
struct Renderable** SC_GetAllRenderables(struct Scene* scene, size_t* num);



void SC_Update(struct Scene* scene, float dt);
void SC_UpdateFrame(struct Scene* scene, float dt);