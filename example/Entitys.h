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


SceneObject* CreateBaseObject(Scene* scene);
SceneObject* CreateBallObject(Scene* scene, const glm::vec2& pos, float size);
SceneObject* CreatePegObject(Scene* scene, const glm::vec2& pos, float size);