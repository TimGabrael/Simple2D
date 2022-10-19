#include "Entitys.h"
#include "GameManager.h"
#include <string>

static constexpr SPRITES sprites[ENTITY_TYPE::NUM_ENTITYS] = {
		SPRITES::DISH_2, SPRITES::DISH_2,
		SPRITES::PIZZA, SPRITES::BURGER, SPRITES::DONUT,
};



struct BaseRenderable : public Renderable
{
	BaseRenderable(const glm::vec2& sBound, const glm::vec2& eBound) {
		this->startBound = sBound;
		this->endBound = eBound;
	}
	virtual ~BaseRenderable() = default;
	virtual void AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds)
	{
		uint32_t cur = verts.size();

		GameManager* m = GM_GetGameManager();


		verts.push_back({ {m->vpStart.x, m->vpStart.y }, {0.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {startBound.x, m->vpStart.y }, {1.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {startBound.x, m->vpEnd.y }, {1.0f, 0.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpStart.x, m->vpEnd.y }, {0.0f, 0.0f}, 0x60FFFFFF });

		inds.push_back(cur);
		inds.push_back(cur + 1);
		inds.push_back(cur + 2);
		inds.push_back(cur + 2);
		inds.push_back(cur + 3);
		inds.push_back(cur);

		cur = verts.size();
		verts.push_back({ {endBound.x, m->vpStart.y }, {0.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpEnd.x, m->vpStart.y }, {1.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpEnd.x, m->vpEnd.y }, {1.0f, 0.0f}, 0x60FFFFFF });
		verts.push_back({ {endBound.x, m->vpEnd.y }, {0.0f, 0.0f}, 0x60FFFFFF });

		inds.push_back(cur);
		inds.push_back(cur + 1);
		inds.push_back(cur + 2);
		inds.push_back(cur + 2);
		inds.push_back(cur + 3);
		inds.push_back(cur);


		cur = verts.size();
		verts.push_back({ {m->vpStart.x,endBound.y }, {0.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpEnd.x, endBound.y }, {1.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpEnd.x, m->vpEnd.y }, {1.0f, 0.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpStart.x, m->vpEnd.y }, {0.0f, 0.0f}, 0x60FFFFFF });

		inds.push_back(cur);
		inds.push_back(cur + 1);
		inds.push_back(cur + 2);
		inds.push_back(cur + 2);
		inds.push_back(cur + 3);
		inds.push_back(cur);


	}
	virtual void UpdateFromBody(b2Body* body) {};
	virtual int GetLayer() const
	{
		return 0;
	}
	virtual GLuint GetTexture() const
	{
		return 0;
	}
	virtual GLuint GetFlags() const
	{
		return 0;
	}

	glm::vec2 startBound;
	glm::vec2 endBound;
};


Base::Base(const glm::vec2& st, const glm::vec2& sbound, const glm::vec2& ebound)
{
	startPos = st;
	startBound = sbound;
	endBound = ebound;
	entXStart = startBound.x - 0.6f;
	entYStart = endBound.y + 0.2f;
	xSteps = 0.45f;
	numInRow = 10;
	this->rend = new BaseRenderable(startBound, endBound);

	{
		GameState* game = GetGameState();
		b2BodyDef body{};
		body.type = b2_staticBody;
		body.position = { startBound.x - 0.5f, 0.0f };
		body.angle = 0.0f;
		body.enabled = true;
		body.allowSleep = true;
		body.fixedRotation = false;
		body.gravityScale = 1.0f;
		b2Body* collLeft = game->physics->world.CreateBody(&body);

		b2PolygonShape shape;
		shape.SetAsBox(0.5f, 4.0f);

		b2FixtureDef fixture{};
		fixture.density = 1.0f;
		fixture.friction = 0.1f;
		fixture.isSensor = false;
		fixture.restitution = 0.1f;
		fixture.restitutionThreshold = 0.0f;
		fixture.shape = &shape;

		collLeft->CreateFixture(&fixture);
		left = collLeft;


		body.position = { endBound.x + 0.5f, 0.0f };
		b2Body* collRight = game->physics->world.CreateBody(&body);
		collRight->CreateFixture(&fixture);
		right = collRight;


		body.position = { 0.0f, endBound.y + 0.5f };
		b2Body* collTop = game->physics->world.CreateBody(&body);

		shape.SetAsBox(4.0f, 0.5f);
		collTop->CreateFixture(&fixture);
		top = collTop;
	}

	left->GetUserData().pointer = (uintptr_t)this;
	right->GetUserData().pointer = (uintptr_t)this;
	top->GetUserData().pointer = (uintptr_t)this;
}
Base::~Base()
{
	delete rend;
	PhysicsScene* phys = GetGameState()->physics;
	phys->world.DestroyBody(left);
	phys->world.DestroyBody(right);
	phys->world.DestroyBody(top);
	left = nullptr;
	right = nullptr;
	top = nullptr;
}
void Base::Update(float dt)
{
	GameState* s = GetGameState();
	GameManager* m = GM_GetGameManager();
	bool found = true;
	while (found)
	{
		found = false;
		for (uint32_t i = 0; i < m->ballList.size(); i++)
		{
			if (m->ballList.at(i)->body->GetPosition().y < -2.1f)
			{
				found = true;
				delete m->ballList.at(i);
				m->ballList.erase(m->ballList.begin() + i);
				break;
			}
		}
	}
}
ENTITY_TYPE Base::GetType() const
{
	return ENTITY_TYPE::BASE;
}
void Base::OnCollideWithBall(struct Ball* ball, b2Fixture* fixture, const glm::vec2& normal)
{
	ball->body->ApplyForceToCenter({ normal.x * 0.2f, normal.y * 0.2f }, true);
}



Ball::Ball(const glm::vec2& pos, const glm::vec2& velocity, float size) 
	: quad(pos, glm::vec2(size), GM_GetGameManager()->atlas->bounds[SPRITES::DISH_2].start, GM_GetGameManager()->atlas->bounds[SPRITES::DISH_2].end, GM_GetGameManager()->atlas->texture.uniform, 0xFFFFFFFF, 2)
{
	GameState* game = GetGameState();
	b2BodyDef bodyDef{};
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = { pos.x, pos.y };
	bodyDef.angle = 0.0f;
	bodyDef.enabled = true;
	bodyDef.allowSleep = false;
	bodyDef.fixedRotation = false;
	bodyDef.gravityScale = 1.0f;
	body = game->physics->world.CreateBody(&bodyDef);

	b2CircleShape shape;
	shape.m_radius = size;


	b2FixtureDef fixture{};
	fixture.density = 1.0f;
	fixture.friction = 0.1f;
	fixture.isSensor = false;
	fixture.restitution = 0.5f;
	fixture.restitutionThreshold = 2.0f;
	fixture.shape = &shape;
	fixture.filter.maskBits = 1;
	fixture.filter.categoryBits = 2;

	body->CreateFixture(&fixture);
	body->SetLinearVelocity({ velocity.x, velocity.y });
	body->GetUserData().pointer = (uintptr_t)this;
	simulated = false;
}
Ball::~Ball()
{
	GetGameState()->physics->world.DestroyBody(body);
	body = nullptr;
}
ENTITY_TYPE Ball::GetType() const
{
	return ENTITY_TYPE::BALL;
}
void Ball::OnCollideWithBall(struct Ball* ball, b2Fixture* fixture, const glm::vec2& normal)
{
}
void Ball::UpdateFrame(float dt) 
{
	GM_AddParticle(quad.pos, glm::vec2(0.0f), quad.halfSize, quad.halfSize * 0.1f, 0xFFFFFFFF, 0xFFFFFFFF, SPRITES::DISH_2, 0.0f, 0.0f, 0.1f);
	quad.UpdateFromBody(body);
}

Peg::Peg(const glm::vec2& pos, float size, ENTITY_TYPE type) : 
	quad(pos, glm::vec2(size), GM_GetGameManager()->atlas->bounds[sprites[type]].start, GM_GetGameManager()->atlas->bounds[sprites[type]].end, GM_GetGameManager()->atlas->texture.uniform, 0xFFFFFFFF, 1)
{
	GameState* game = GetGameState();
	b2BodyDef bodyDef{};
	bodyDef.type = b2_staticBody;
	bodyDef.position = { pos.x, pos.y };
	bodyDef.angle = 0.0f;
	bodyDef.enabled = true;
	bodyDef.allowSleep = true;
	bodyDef.fixedRotation = false;
	bodyDef.gravityScale = 1.0f;
	body = game->physics->world.CreateBody(&bodyDef);

	b2CircleShape shape;
	shape.m_radius = size;

	b2FixtureDef fixture{};
	fixture.density = 1.0f;
	fixture.friction = 0.1f;
	fixture.isSensor = false;
	fixture.restitution = 0.1f;
	fixture.restitutionThreshold = 0.0f;
	fixture.shape = &shape;
	body->CreateFixture(&fixture);
	body->GetUserData().pointer = (uintptr_t)this;
	flags = 0;
	this->type = type;
}
Peg::~Peg()
{
	GetGameState()->physics->world.DestroyBody(body);
	body = nullptr;
}

void Peg::UpdatePegType(ENTITY_TYPE type)
{
	GameManager* m = GM_GetGameManager();
	this->type = type;
	AtlasTexture::UVBound& bound = m->atlas->bounds[sprites[type]];
	quad.uvStart = bound.start;
	quad.uvEnd = bound.end;
}
ENTITY_TYPE Peg::GetType() const
{
	return type;
}
void Peg::OnCollideWithBall(Ball* ball, b2Fixture* fixture, const glm::vec2& normal)
{
	ball->body->ApplyForceToCenter({ normal.x * 1.0f, normal.y * 1.0f }, true);
	if (!ball->simulated)
	{
		GameManager* m = GM_GetGameManager();
		switch (type)
		{
		case STANDARD_PEG:
			m->stats.accumulatedDamage += 1.0f * m->stats.critMultiplier;
			SetInactive();
			break;
		case REFRESH_PEG:
			m->stats.accumulatedDamage += 1.0f * m->stats.critMultiplier;
			SetInactive();
			for (uint32_t i = 0; i < m->pegList.size(); i++)
			{
				Peg* p = m->pegList.at(i);
				if (p->flags & PEG_FLAGS::INACTIVE)
				{
					p->UpdatePegType(GM_GenerateRandomPegType());
					p->SetActive();
				}
			}
			break;
		case CRIT_PEG:
		{
			SetInactive();
			float oldCritMultiplier = m->stats.critMultiplier;
			m->stats.critMultiplier = std::min(m->stats.critMultiplier + 1.0f, m->stats.maxCritMultiplier);
			float diffMultiplier = m->stats.critMultiplier - oldCritMultiplier;
			m->stats.accumulatedDamage += m->stats.accumulatedDamage * diffMultiplier;
			break;
		}
		default:
			break;
		};
		
		b2Vec2 pos = body->GetPosition();
		GM_AddParticle({ pos.x, pos.y }, glm::vec2(0.0f), glm::vec2(quad.halfSize), glm::vec2(quad.halfSize * 2.0f), 0xFFFFFFFF, 0x60FFFFFF, SPRITES::PIZZA, 0.0f, 0.0f, 0.2f);
		
		GM_PlaySound(SOUNDS::SOUND_CLACK, 0.2f);

		uint32_t textColor = 0xFFFFFF;
		if (m->stats.critMultiplier > 1.0f)
		{
			textColor = 0x00d7FF;
		}
		std::string dmgString = std::to_string((int)m->stats.accumulatedDamage);
		GM_AddTextParticle(dmgString.c_str(), glm::vec2(pos.x, pos.y + 0.05f), glm::vec2(0.0f, 0.3f), glm::vec2(0.0f, 0.0f), 1.0f, 0.5f, textColor | (0xFF << 24), textColor | (0xA0 << 24), 0.4f);

	}
	else
	{
		body->SetEnabled(false);
	}
}
void Peg::SetInactive()
{
	quad.col = (quad.col & 0xFFFFFF) | (0x80 << 24);
	body->SetEnabled(false);
	flags |= PEG_FLAGS::INACTIVE;
}
void Peg::SetActive()
{
	quad.col = (quad.col & 0xFFFFFF) | (0xFF << 24);
	body->SetEnabled(true);
	flags &= ~PEG_FLAGS::INACTIVE;
}

Projectile::Projectile(const glm::vec2& pos, float size, SPRITES sprite)
	: quad(pos, glm::vec2(size), GM_GetGameManager()->atlas->bounds[sprite].start, GM_GetGameManager()->atlas->bounds[sprite].end, GM_GetGameManager()->atlas->texture.uniform, 0xFFFFFFFF, 1)
{

}
Projectile::~Projectile()
{

}
void Projectile::UpdateFrame(float dt)
{
	GameManager* m = GM_GetGameManager();
	const float move = 6.0f * dt;

	float prevX = quad.pos.x;
	quad.pos.x += move;
	float curX = quad.pos.x;

	for (uint32_t i = 0; i < m->enemyList.size(); i++)
	{
		AnimatedQuad& anim = m->enemyList.at(i)->quad;
		
		const float start = anim.pos.x - anim.halfSize.x;
		if (prevX < start && curX >= start)
		{
			if (!OnHitEnemy(m->enemyList.at(i), i))
			{
				for (uint32_t i = 0; i < m->projList.size(); i++)
				{
					if (m->projList.at(i) == this)
					{
						delete this;
						m->projList.erase(m->projList.begin() + i);
						return;
					}
				}
			}
		}
	}

	if (quad.pos.x + quad.halfSize.x > m->vpEnd.x)
	{
		for (uint32_t i = 0; i < m->projList.size(); i++)
		{
			if (m->projList.at(i) == this)
			{
				delete this;
				m->projList.erase(m->projList.begin() + i);
				return;
			}
		}
	}
}
// RETURN TRUE IF THE PROJECTILE SHOULD CONTINUE TO TRAVEL
bool Projectile::OnHitEnemy(struct Character* hit, uint32_t idx)
{
	hit->ApplyDamage(SOUNDS::SOUND_NONE, SOUNDS::SOUND_SLIME_DIE, GM_GetGameManager()->stats.accumulatedDamage);
	return false;
}


Character::Character(const glm::vec2& pos, float size)
	: quad(pos, glm::vec2(size), GM_GetGameManager()->atlas, 0xFFFFFFFF, 1)
{

}
Character::~Character()
{

}
void Character::UpdateAnimation(float dt)
{
	static constexpr float animStepTime = 1.0f / 10.0f;
	this->animTimer += dt;
	while (animTimer >= animStepTime)
	{
		animIdx++;
		animTimer -= animStepTime;
	}
}
void Character::SetAnimation(ANIMATION anim)
{
	playAnimOnce = false;
	if (anim != IDLE && anim != MOVE) {
		playAnimOnce = true;
		if (activeAnimation == anim) return;
	}
	animIdx = 0;
	activeAnimation = anim;
}
void Character::ApplyDamage(enum SOUNDS hurt, enum SOUNDS die, int dmg)
{
	health = glm::max(health - dmg, 0);
	if (health <= 0)
	{
		GM_PlaySound(die, 1.0f);
		SetAnimation(DIE);
	}
	else
	{
		GM_PlaySound(hurt, 1.0f);
		SetAnimation(HURT);
	}
}
Player::Player(const glm::vec2& pos, float size) : Character(pos, size)
{
	quad.AddAnimRange(SPRITES::FOX_IDLE_0, SPRITES::FOX_IDLE_4 + 1);
	quad.AddAnimRange(SPRITES::FOX_LEAP_0, SPRITES::FOX_LEAP_10 + 1);
	quad.AddAnimRange(SPRITES::FOX_SHOCK_0, SPRITES::FOX_SHOCK_4 + 1);
	quad.AddAnimRange(SPRITES::FOX_LAY_0, SPRITES::FOX_LAY_6 + 1);
	quad.AddAnimRange(SPRITES::FOX_WALK_0, SPRITES::FOX_WALK_7 + 1);
}
Player::~Player()
{

}
void Player::UpdateFrame(float dt)
{
	UpdateAnimation(dt);
	
	quad.animIdx = activeAnimation;
	if (this->activeAnimation < quad.range.size())
	{
		uint32_t oldAnimIdx = animIdx;
		auto range = quad.range.at(activeAnimation);
		animIdx = (animIdx % (range.endIdx - range.startIdx));
		if (animIdx < oldAnimIdx)
		{
			if (playAnimOnce)
			{
				animIdx = 0;
				activeAnimation = 0;
				playAnimOnce = false;
			}
		}
		quad.animStepIdx = animIdx;
	}
	
}
void Player::BeginAction()
{

}
bool Player::PerformAction(float dt)
{
	return true;
}

Slime::Slime(const glm::vec2& pos, float size) : Character(pos, size)
{
	quad.AddAnimRange(SLIME_IDLE_0, SLIME_IDLE_3 + 1);
	quad.AddAnimRange(SLIME_ATTACK_0, SLIME_ATTACK_4 + 1);
	quad.AddAnimRange(SLIME_HURT_0, SLIME_HURT_3 + 1);
	quad.AddAnimRange(SLIME_DIE_0, SLIME_DIE_3 + 1);
	quad.AddAnimRange(SLIME_MOVE_0, SLIME_MOVE_3 + 1);
}
Slime::~Slime()
{

}
void Slime::UpdateFrame(float dt)
{
	uint32_t prevAnimIdx = animIdx;
	UpdateAnimation(dt);

	quad.animIdx = activeAnimation;
	if (this->activeAnimation < quad.range.size())
	{
		uint32_t oldAnimIdx = animIdx;
		auto range = quad.range.at(activeAnimation);
		animIdx = (animIdx % (range.endIdx - range.startIdx));
		if (activeAnimation == ATTACK)
		{
			if (animIdx <= 2 && oldAnimIdx >= 3)
			{
				GM_GetGameManager()->player->ApplyDamage(SOUNDS::SOUND_NONE, SOUNDS::SOUND_NONE, 10);
			}
		}
		if (animIdx < oldAnimIdx)
		{
			if (playAnimOnce)
			{
				if (activeAnimation == DIE) {
					GameManager* m = GM_GetGameManager();
					for (uint32_t i = 0; i < m->enemyList.size(); i++)
					{
						delete m->enemyList.at(i);
						m->enemyList.erase(m->enemyList.begin() + i);
						return;
					}
				}
				else
				{
					animIdx = 0;
					activeAnimation = 0;
					playAnimOnce = false;
				}
			}
		}
		quad.animStepIdx = animIdx;
	}
	
}
void Slime::BeginAction()
{
	GameManager* m = GM_GetGameManager();

	float curMinAhead = FLT_MAX;
	Character* ahead = nullptr;
	for (uint32_t i = 0; i < m->enemyList.size(); i++)
	{
		if (m->enemyList.at(i) == this) continue;
		const glm::vec2 otherPos = m->enemyList.at(i)->quad.pos;
		const float dist = (quad.pos.x - otherPos.x);

		if (dist > 0.0f && dist < curMinAhead)
		{
			curMinAhead = dist;
			ahead = m->enemyList.at(i);
		}
	}
	float desiredStep = 0.0f;
	if (ahead)
	{
		float finalPos = ahead->quad.pos.x + ahead->quad.halfSize.x + quad.halfSize.x;
		desiredStep = glm::min(m->background->xSteps, quad.pos.x - finalPos);
		canAttack = false;
	}
	else
	{
		float finalPos = m->background->entXStart + m->player->quad.halfSize.x + quad.halfSize.x;
		desiredStep = glm::min(m->background->xSteps, quad.pos.x - finalPos);

		canAttack = true;
	}
	if (desiredStep < 0.001f) targetXPos = 100000.0f;
	else targetXPos = quad.pos.x - desiredStep;
}
bool Slime::PerformAction(float dt)
{
	if (activeAnimation == Character::HURT || activeAnimation == Character::DIE) return false;
	if (health <= 0) {
		SetAnimation(Character::DIE);
		return false;
	}
	if (targetXPos < 1000.0f)
	{
		canAttack = false;
		SetAnimation(Character::MOVE);
		quad.pos.x = quad.pos.x - 1.0f * dt;
		if (quad.pos.x <= targetXPos)
		{
			quad.pos.x = targetXPos;
			targetXPos = 100000.0f;
		}
	}
	else
	{
		if (canAttack && activeAnimation != Character::ATTACK)
		{
			GM_PlaySound(SOUNDS::SOUND_SLIME_ATTACK, 1.0f);
			SetAnimation(Character::ATTACK);
			canAttack = false;
		}
		else
		{
			if (activeAnimation == Character::ATTACK) return false;
			SetAnimation(Character::IDLE);
			return true;
		}
	}
	return false;
}

ParticleHandlerEntity::ParticleHandlerEntity()
	: base(GM_GetGameManager()->atlas, 5000)
{

}
ParticleHandlerEntity::~ParticleHandlerEntity()
{

}



std::vector<glm::vec2> SimulateBall(const glm::vec2& pos, const glm::vec2& velocity, float size, float simulateDuration)
{
	GameState* state = GetGameState();
	GameManager* m = (GameManager*)state->manager;
	if (m->ballList.size() > 0) return {};

	std::vector<glm::vec2> accumulated = {pos};

	Ball* simBall = new Ball(pos, velocity, size);
	simBall->simulated = true;
	m->ballList.push_back(simBall);

	while (simulateDuration >= TIME_STEP)
	{
		UpdateGameStep();
		if (m->ballList.size() == 0) 
		{
			break;
		}
		else
		{
			auto p = m->ballList.at(0)->body->GetPosition();
			accumulated.push_back({ p.x, p.y });
		}
		simulateDuration -= TIME_STEP;
	}

	if (m->ballList.size() > 0)
	{
		auto p = m->ballList.at(0)->body->GetPosition();
		accumulated.push_back({ p.x, p.y });
		for (Ball* b : m->ballList)
		{
			delete b;
		}
		m->ballList.clear();
	}

	for (uint32_t i = 0; i < m->pegList.size(); i++)
	{
		if (m->pegList.at(i)->flags & Peg::PEG_FLAGS::INACTIVE) continue;
		m->pegList.at(i)->body->SetEnabled(true);
	}

	return accumulated;
}

Character* CreateEnemy(const glm::vec2& pos, float size, CHARACTER_TYPES c)
{
	Character* res = nullptr;
	if (c == CHARACTER_TYPES::SLIME)
	{
		glm::vec2 realPos = { pos.x, pos.y - 0.02f };	// the slime sprite is not perfectly sized inside the bounding rectangle
		res = new Slime(realPos, size);
	}
	return res;
}