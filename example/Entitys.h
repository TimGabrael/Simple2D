#pragma once
#include "GameState.h"
#include "GameManager.h"

enum ENTITY_TYPE
{
	BASE,
	BALL,
	STANDARD_PEG,
};

struct PeggleEntity : public Entity
{
	virtual ENTITY_TYPE GetType() const = 0;
	virtual void OnCollideWithBall(struct SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal) = 0;

	SceneObject* obj = nullptr;
};

struct Base : public PeggleEntity
{
	Base();

	virtual ~Base();

	virtual void Update(float dt);
	virtual ENTITY_TYPE GetType() const override;
	virtual void OnCollideWithBall(struct SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal) override;



	b2Body* left;
	b2Body* right;
	b2Body* top;
	glm::vec2 startPos;
	glm::vec2 startBound;
	glm::vec2 endBound;
};

struct Ball : public PeggleEntity
{
	Ball(const glm::vec2& pos);

	virtual void Update(float dt) {}

	virtual ENTITY_TYPE GetType() const override;
	virtual void OnCollideWithBall(struct SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal) override;
	glm::vec2 pos;
};

struct Peg : public PeggleEntity
{
	Peg(const glm::vec2& pos);

	virtual void Update(float dt) {}

	virtual ENTITY_TYPE GetType() const override;
	virtual void OnCollideWithBall(struct SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal) override;

	glm::vec2 pos;
};


std::vector<glm::vec2> SimulateBall(const glm::vec2& pos, const glm::vec2& velocity, float size, float simulateDuration);

SceneObject* CreateBaseObject(Scene* scene);
SceneObject* CreateBallObject(Scene* scene, const glm::vec2& pos, const glm::vec2& velocity, float size);
SceneObject* CreatePegObject(Scene* scene, const glm::vec2& pos, float size);

void RemoveBaseObject(SceneObject* obj);
void RemoveBallObject(size_t idx);
void RemovePegObject(size_t idx);

void RemoveAllBalls();
void RemoveAllPegs();
void RemoveAllObjects();