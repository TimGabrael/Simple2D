#pragma once
#include "GameState.h"

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
	virtual ~PeggleEntity() = default;
	virtual ENTITY_TYPE GetType() const = 0;
	virtual void OnCollideWithBall(struct Ball* ball, b2Fixture* fixture, const glm::vec2& normal) = 0;
};

struct Base : public PeggleEntity
{
	Base(const glm::vec2& st, const glm::vec2& sbound, const glm::vec2& ebound);
	virtual ~Base();

	virtual void Update(float dt);
	virtual void UpdateFrame(float dt) { }
	virtual ENTITY_TYPE GetType() const override;
	virtual void OnCollideWithBall(struct Ball* ball, b2Fixture* fixture, const glm::vec2& normal) override;

	b2Body* left;
	b2Body* right;
	b2Body* top;
	struct BaseRenderable* rend;
	glm::vec2 startPos;
	glm::vec2 startBound;
	glm::vec2 endBound;
	float entXStart;
	float entYStart;
	float xSteps;
	uint32_t numInRow;
};

struct Ball : public PeggleEntity
{
	Ball(const glm::vec2& pos, const glm::vec2& velocity, float size);
	virtual ~Ball();
	virtual void Update(float dt){}
	virtual void UpdateFrame(float dt);

	virtual ENTITY_TYPE GetType() const override;
	virtual void OnCollideWithBall(struct Ball* ball, b2Fixture* fixture, const glm::vec2& normal) override;
	b2Body* body;
	TextureQuad quad;
	bool simulated;
};

struct Peg : public PeggleEntity
{
	enum PEG_FLAGS
	{
		INACTIVE = 1,
	};
	Peg(const glm::vec2& pos, float size, ENTITY_TYPE type);
	virtual ~Peg();
	virtual void Update(float dt) {}
	virtual void UpdateFrame(float dt) { }

	void UpdatePegType(ENTITY_TYPE type);

	virtual ENTITY_TYPE GetType() const override;
	virtual void OnCollideWithBall(struct Ball* ball, b2Fixture* fixture, const glm::vec2& normal) override;

	void SetInactive();
	void SetActive();

	b2Body* body;
	TextureQuad quad;
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
	Projectile(const glm::vec2& pos, float size, enum SPRITES sprite);
	virtual ~Projectile();
	enum Flags
	{
		DAMAGE_PASS_THROUGH = 1,
		SINGLE_TARGET = 2,
		MULTI_TARGET = 4,
		ENEMY_PROJECTILE = 8,
	};
	virtual void UpdateFrame(float dt);
	bool OnHitEnemy(struct Character* hit, uint32_t idx);
	virtual void Update(float dt) { }

	TextureQuad quad;
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
	Character(const glm::vec2& pos, float size);
	virtual ~Character();
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

	virtual void BeginAction() = 0;
	virtual bool PerformAction(float dt) = 0;

	void ApplyDamage(enum SOUNDS hurt, enum SOUNDS die, int dmg);
	
	AnimatedQuad quad;
	float animTimer = 0.0f;
	uint32_t animIdx = 0;
	int health = 100;
	int maxHealth = 100;
	uint32_t activeAnimation = 0;
	bool playAnimOnce = false;
};

struct Player : public Character
{
	Player(const glm::vec2& pos, float size);
	~Player();
	virtual void Update(float dt) {};
	virtual void UpdateFrame(float dt) override;

	virtual void BeginAction() override;
	virtual bool PerformAction(float dt) override;
};

struct Slime : public Character
{
	Slime(const glm::vec2& pos, float size);
	~Slime();

	virtual void BeginAction() override;
	virtual bool PerformAction(float dt) override;

	virtual void Update(float dt) {};
	virtual void UpdateFrame(float dt) override;
	float targetXPos;
	bool canAttack;
};

struct ParticleHandlerEntity : public Entity
{
	ParticleHandlerEntity();
	virtual ~ParticleHandlerEntity();
	virtual void Update(float dt) {};
	virtual void UpdateFrame(float dt) {
		base.Update(dt);
	}
	ParticlesBase base;
};



std::vector<glm::vec2> SimulateBall(const glm::vec2& pos, const glm::vec2& velocity, float size, float simulateDuration);


Character* CreateEnemy(const glm::vec2& pos, float size, CHARACTER_TYPES c);