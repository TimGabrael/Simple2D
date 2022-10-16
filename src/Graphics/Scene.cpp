#include "Scene.h"
#include "GameState.h"
#define NUM_OBJECTS_IN_LIST sizeof(uintptr_t) * 8

struct SceneObjectList
{
	SceneObjectList* next;
	uintptr_t fillMask;
	SceneObject objects[NUM_OBJECTS_IN_LIST];
};

struct Scene
{
	SceneObjectList objects;
	uint32_t numObjects;
	SceneObject** cachedList;
	uint32_t cachedListCapacity;
	bool needRebuild;
};

static void FreeObject(SceneObject* obj)
{
	GameState* game = GetGameState();
	if (obj->entity) delete obj->entity;
	if (obj->body) game->physics->world.DestroyBody(obj->body);
	if (obj->renderable) delete obj->renderable;
	obj->entity = nullptr;
	obj->body = nullptr;
	obj->renderable = nullptr;
}


static SceneObject* AllocateObjectFromList(SceneObjectList* list)
{
	if (list->fillMask == 0xFFFFFFFFFFFFFFFFLL)
	{
		if (list->next)
		{
			return AllocateObjectFromList(list->next);
		}
		else
		{
			list->next = new SceneObjectList;
			memset(list->next, 0, sizeof(SceneObjectList));
			list->next->fillMask = 1;
			return &list->next->objects[0];
		}
	}
	else
	{
		for (uint64_t i = 0; i < NUM_OBJECTS_IN_LIST; i++)
		{
			uint64_t cur = (1LL << i);
			if (!(list->fillMask & cur))
			{
				list->fillMask |= cur;
				return &list->objects[i];
			}
		}
	}
	return nullptr;
}
static void FreeObjectFromSubList(SceneObjectList* oldList, SceneObjectList* list, SceneObject* obj)
{
	const uintptr_t idx = ((uintptr_t)obj - (uintptr_t)&list->objects[0]) / sizeof(SceneObject);
	if (idx >= 0 && idx < NUM_OBJECTS_IN_LIST)
	{
		const uintptr_t bit = (1LL << idx);
		if (list->fillMask & bit)
		{
			list->fillMask &= ~bit;
			FreeObject(&list->objects[idx]);
		}

		if (list->fillMask == 0) // DELETE THE LIST AS IT DOES NOT CONTAIN ANY OBJECTS ANYMORE!!!
		{
			oldList->next = list->next;
			delete list;
		}
	}
	else
	{
		FreeObjectFromSubList(list, list->next, obj);
	}
}
static void FreeObjectFromList(SceneObjectList* list, SceneObject* obj)
{
	const uintptr_t idx = ((uintptr_t)obj - (uintptr_t)&list->objects[0]) / sizeof(SceneObject);
	if (idx >= 0 && idx < NUM_OBJECTS_IN_LIST)
	{
		const uintptr_t bit = (1LL << idx);
		if (list->fillMask & bit)
		{
			list->fillMask &= ~bit;
			FreeObject(&list->objects[idx]);
		}
	}
	else
	{
		FreeObjectFromSubList(list, list->next, obj);
	}
}

// THIS FUNCTION DOES NOT ITERATE OVER ALL CONNECTED LISTS!!!
static void FreeObjectIfExists(SceneObjectList* list, int idx)
{
	const uintptr_t bit = (1LL << idx);
	if (list->fillMask & bit)
	{
		list->fillMask &= ~bit;
		FreeObject(&list->objects[idx]);
	}
}

static void FreeEntireList(SceneObjectList* list)
{
	while (list->next)
	{
		for (int i = 0; i < NUM_OBJECTS_IN_LIST; i++)
		{
			FreeObjectIfExists(list->next, i);
		}
		SceneObjectList* old = list->next;
		list->next = old->next;
		delete old;
	}
	for (int i = 0; i < NUM_OBJECTS_IN_LIST; i++)
	{
		FreeObjectIfExists(list, i);
	}
}



struct Scene* SC_CreateScene()
{
	Scene* scene = new Scene;
	memset(scene, 0, sizeof(Scene));
	scene->needRebuild = true;
	return scene;
}
void SC_CleanUpScene(struct Scene* scene)
{
	SC_RemoveAll(scene);
	if (scene->cachedList)
	{
		delete[] scene->cachedList;
		scene->cachedList = nullptr;
		scene->cachedListCapacity = 0;
	}
	delete scene;
}

void SC_RemoveAll(struct Scene* scene)
{
	FreeEntireList(&scene->objects);
	scene->numObjects = 0;
	scene->needRebuild = true;
}

SceneObject* SC_AddObject(struct Scene* scene, const SceneObject* obj)
{
	SceneObject* out = AllocateObjectFromList(&scene->objects);
	scene->numObjects = scene->numObjects + 1;
	memcpy(out, obj, sizeof(SceneObject));
	out->flags = OBJECT_FLAG_VISIBLE;
	return out;
}
void SC_RemoveObject(struct Scene* scene, SceneObject* obj)
{
	FreeObjectFromList(&scene->objects, obj);
	scene->numObjects = scene->numObjects - 1;
	scene->needRebuild = true;
}

SceneObject** SC_GetAllSceneObjects(struct Scene* scene, uint32_t* num)
{
	const uint32_t totalNum = scene->numObjects;
	*num = totalNum;
	if (scene->needRebuild)
	{
		if (scene->cachedListCapacity <= totalNum)
		{
			if (scene->cachedList) delete[] scene->cachedList;
			scene->cachedListCapacity = totalNum + 10;
			scene->cachedList = new SceneObject * [scene->cachedListCapacity];
		}

		uint32_t curIdx = 0;
		SceneObjectList* curList = &scene->objects;
		while (curList)
		{
			for (int i = 0; i < NUM_OBJECTS_IN_LIST; i++)
			{
				const uintptr_t remaining = ~((1LL << i) - 1 | (1LL << i));
				if (curList->fillMask & (1LL << i))
				{
					scene->cachedList[curIdx] = &curList->objects[i];
					curIdx++;
				}
				if (!(remaining & curList->fillMask)) break;
			}
			curList = curList->next;
		}
	}
	return scene->cachedList;
}

void SC_Update(struct Scene* scene, float dt)
{
	uint32_t num = 0;
	SceneObject** obj = SC_GetAllSceneObjects(scene, &num);
	for (int i = 0; i < num; i++)
	{
		if (obj[i]->entity)
		{
			obj[i]->entity->Update(dt);
		}
	}
	// rebuild in case any objects were created in the entity update function
	SC_GetAllSceneObjects(scene, &num);
}
