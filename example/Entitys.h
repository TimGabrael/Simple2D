#pragma once
#include "GameState.h"
#include "GameManager.h"

enum ENTITY_TYPE
{
	BASE,
	BALL,
	STANDARD_PEG,
	REFRESH_PEG,
	NUM_ENTITYS,
};

struct PeggleEntity : public Entity
{
	virtual ENTITY_TYPE GetType() const = 0;
	virtual void OnCollideWithBall(struct SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal) = 0;

	SceneObject* obj = nullptr;
};

struct Base : public PeggleEntity
{
	Base(const glm::vec2& st, const glm::vec2& sbound, const glm::vec2& ebound);
	virtual ~Base();

	virtual void Update(float dt);
	virtual void UpdateFrame(float dt) { }
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
	virtual ~Ball() = default;
	virtual void Update(float dt) {}
	virtual void UpdateFrame(float dt) {}

	virtual ENTITY_TYPE GetType() const override;
	virtual void OnCollideWithBall(struct SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal) override;
	glm::vec2 pos;
};

struct Peg : public PeggleEntity
{
	enum PEG_FLAGS
	{
		INACTIVE = 1,
	};
	Peg(const glm::vec2& pos, ENTITY_TYPE type);
	virtual ~Peg() = default;
	virtual void Update(float dt) {}
	virtual void UpdateFrame(float dt) { }

	void UpdatePegType(ENTITY_TYPE type);

	virtual ENTITY_TYPE GetType() const override;
	virtual void OnCollideWithBall(struct SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal) override;

	void SetInactive();
	void SetActive();

	glm::vec2 pos;
	uint32_t flags;
	ENTITY_TYPE type;
};





enum PROJECTILE_TYPES
{
	BASE_PROJECTILE,
	NUM_PROJECTILES,
};

struct Projectile : public Entity
{
	enum Flags
	{
		DAMAGE_PASS_THROUGH,
		SINGLE_TARGET,
		MULTI_TARGET,
	};
	virtual void UpdateFrame(float dt);
	bool OnHitEnemy(struct Character* hit, uint32_t idx);
	virtual void Update(float dt) { }

	SceneObject* obj;
	PROJECTILE_TYPES type;
	uint32_t flags;
	int target;
};




enum CHARACTER_TYPES
{
	PLAYER,
	SLIME,
	NUM_CHARACTERS,
};


struct Character : public Entity
{
	enum ANIMATION
	{
		IDLE,
		ATTACK,
		HURT,
		DIE,
		MOVE,
	};
	void UpdateAnimation(float dt);
	void SetAnimation(ANIMATION anim);
	SceneObject* obj = nullptr;
	float animTimer = 0.0f;
	uint32_t animIdx = 0;
	int health = 100;
	uint32_t activeAnimation = 0;
	bool playAnimOnce = false;
};

struct Player : public Character
{
	~Player() = default;
	virtual void Update(float dt) {};
	virtual void UpdateFrame(float dt) override;
};

struct Slime : public Character
{
	~Slime() = default;
	virtual void Update(float dt) {};
	virtual void UpdateFrame(float dt) override;
};





std::vector<glm::vec2> SimulateBall(const glm::vec2& pos, const glm::vec2& velocity, float size, float simulateDuration);

SceneObject* CreateBaseObject(Scene* scene);
SceneObject* CreateBallObject(Scene* scene, const glm::vec2& pos, const glm::vec2& velocity, float size);
SceneObject* CreatePegObject(Scene* scene, const glm::vec2& pos, float size, ENTITY_TYPE type);
SceneObject* CreatePlayerObject(Scene* scene, const glm::vec2& pos, float size);
SceneObject* CreateProjectileObject(Scene* scene, const glm::vec2& pos, float size);
SceneObject* CreateParticlesBaseObject(Scene* scene);
SceneObject* CreateEnemyObject(Scene* scene, const glm::vec2& pos, float size, CHARACTER_TYPES c);

void RemoveBaseObject();
void RemoveBallObject(size_t idx);
void RemovePegObject(size_t idx);
void RemovePlayerObject();
void RemoveProjectileObject(size_t idx);
void RemoveParticlesBaseObject();
void RemoveEnemyObject(uint32_t idx);

void RemoveAllBalls();
void RemoveAllPegs();
void RemoveAllProjectiles();
void RemoveAllEnemyObjects();
void RemoveAllObjects();