#include "Entitys.h"


struct BaseRenderable : public Renderable
{
	BaseRenderable() {
		this->texture = 0;// GM_GetGameManager()->atlas->texture.uniform;
		this->layer = 0;
	}
	virtual ~BaseRenderable() = default;
	virtual void AddVertices(std::vector<Vertex2D>& verts, std::vector<uint32_t>& inds)
	{
		uint32_t cur = verts.size();

		GameManager* m = GM_GetGameManager();


		verts.push_back({ {m->vpStart.x, m->vpStart.y }, {0.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {-1.5f, m->vpStart.y }, {1.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {-1.5f, m->vpEnd.y }, {1.0f, 0.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpStart.x, m->vpEnd.y }, {0.0f, 0.0f}, 0x60FFFFFF });

		inds.push_back(cur);
		inds.push_back(cur + 1);
		inds.push_back(cur + 2);
		inds.push_back(cur + 2);
		inds.push_back(cur + 3);
		inds.push_back(cur);

		cur = verts.size();
		verts.push_back({ {1.5f, m->vpStart.y }, {0.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpEnd.x, m->vpStart.y }, {1.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpEnd.x, m->vpEnd.y }, {1.0f, 0.0f}, 0x60FFFFFF });
		verts.push_back({ {1.5f, m->vpEnd.y }, {0.0f, 0.0f}, 0x60FFFFFF });

		inds.push_back(cur);
		inds.push_back(cur + 1);
		inds.push_back(cur + 2);
		inds.push_back(cur + 2);
		inds.push_back(cur + 3);
		inds.push_back(cur);


		cur = verts.size();
		verts.push_back({ {m->vpStart.x, 1.5f }, {0.0f, 1.0f}, 0x60FFFFFF });
		verts.push_back({ {m->vpEnd.x, 1.5f }, {1.0f, 1.0f}, 0x60FFFFFF });
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
};


Base::Base()
{
	startPos = glm::vec2(0.0f, 1.3f);
	startBound = glm::vec2(-1.5f);
	endBound = glm::vec2(1.5f);
}
Base::~Base()
{
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
				SC_RemoveObject(s->scene, m->ballList.at(i));
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
void Base::OnCollideWithBall(struct SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal)
{
	ball->body->ApplyForceToCenter({ normal.x * 0.2f, normal.y * 0.2f }, true);
}




Ball::Ball(const glm::vec2& pos)
{
	this->pos = pos;
}
ENTITY_TYPE Ball::GetType() const
{
	return ENTITY_TYPE::BALL;
}
void Ball::OnCollideWithBall(struct SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal)
{
}

Peg::Peg(const glm::vec2& pos, ENTITY_TYPE type)
{
	this->pos = pos;
	this->type = type;
	flags = 0;
}

ENTITY_TYPE Peg::GetType() const
{
	return type;
}

void Peg::OnCollideWithBall(SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal)
{
	ball->body->ApplyForceToCenter({ normal.x * 0.1f, normal.y * 0.1f }, true);
	if (ball->renderable)
	{
		GameManager* m = GM_GetGameManager();
		switch (type)
		{
		case STANDARD_PEG:
			m->accumulatedDamage += 1;
			SetInactive();
			break;
		case REFRESH_PEG:
			m->accumulatedDamage += 1;
			for (uint32_t i = 0; i < m->pegList.size(); i++)
			{
				((Peg*)m->pegList.at(i)->entity)->SetActive();
			}
			break;
		default:
			break;
		};
		PlaySound(SOUNDS::SOUND_CLACK, 2.0f);
	}
	else
	{
		obj->body->SetEnabled(false);
	}
}
void Peg::SetInactive()
{
	((TextureQuad*)obj->renderable)->col = (((TextureQuad*)obj->renderable)->col & 0xFFFFFF) | (0x80 << 24);
	obj->body->SetEnabled(false);
	flags |= PEG_FLAGS::INACTIVE;
}
void Peg::SetActive()
{
	((TextureQuad*)obj->renderable)->col = (((TextureQuad*)obj->renderable)->col & 0xFFFFFF) | (0xFF << 24);
	obj->body->SetEnabled(true);
	flags &= PEG_FLAGS::INACTIVE;
}


void Projectile::UpdateFrame(float dt)
{
	GameManager* m = GM_GetGameManager();
	const float move = 6.0f * dt;
	TextureQuad* q = (TextureQuad*)obj->renderable;
	q->pos.x += move;
	if (q->pos.x + q->halfSize.x > m->vpEnd.x)
	{
		for (uint32_t i = 0; i < m->projList.size(); i++)
		{
			if (m->projList.at(i)->entity == this)
			{
				RemoveProjectileObject(i);
				return;
			}
		}
	}
}
bool Projectile::OnHitEnemy(struct Character* hit)
{
	hit->health = glm::max(hit->health - 10, 0);
	return true;
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
void Player::UpdateFrame(float dt)
{
	UpdateAnimation(dt);
	if (obj->renderable)
	{
		AnimatedQuad* r = (AnimatedQuad*)obj->renderable;
		r->animIdx = activeAnimation;
		if (this->activeAnimation < r->range.size())
		{
			uint32_t oldAnimIdx = animIdx;
			auto range = r->range.at(activeAnimation);
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
			r->animStepIdx = animIdx;
		}
	}
}
void Player::SetAnimation(ANIMATION anim)
{
	if (anim != IDLE)  playAnimOnce = true;
	animIdx = 0;
	activeAnimation = anim;
}







std::vector<glm::vec2> SimulateBall(const glm::vec2& pos, const glm::vec2& velocity, float size, float simulateDuration)
{
	GameState* state = GetGameState();
	GameManager* m = (GameManager*)state->manager;
	if (m->ballList.size() > 0) return {};

	std::vector<glm::vec2> accumulated = {pos};

	SceneObject* obj = CreateBallObject(state->scene, pos, velocity, size);
	if (obj->renderable)
	{
		delete obj->renderable;
		obj->renderable = nullptr;
	}
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
		for (uint32_t i = 0; i < m->ballList.size(); i++)
		{
			SC_RemoveObject(state->scene, m->ballList.at(i));
		}
		m->ballList.clear();
	}

	for (uint32_t i = 0; i < m->pegList.size(); i++)
	{
		if (((Peg*)m->pegList.at(i)->entity)->flags & Peg::PEG_FLAGS::INACTIVE) continue;
		m->pegList.at(i)->body->SetEnabled(true);
	}

	return accumulated;
}


SceneObject* CreateBaseObject(Scene* scene)
{
	GameState* game = GetGameState();

	Base* base = new Base();
	{
		b2BodyDef body{};
		body.type = b2_staticBody;
		body.position = { base->startBound.x - 0.5f, 0.0f };
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
		base->left = collLeft;


		body.position = { base->endBound.x + 0.5f, 0.0f };
		b2Body* collRight = game->physics->world.CreateBody(&body);
		collRight->CreateFixture(&fixture);
		base->right = collRight;


		body.position = { 0.0f, base->endBound.y + 0.5f };
		b2Body* collTop = game->physics->world.CreateBody(&body);

		shape.SetAsBox(4.0f, 0.5f); 
		collTop->CreateFixture(&fixture);
		base->top = collTop;
		
	}

	SceneObject obj;
	obj.entity = base;
	obj.body = nullptr;
	obj.renderable = new BaseRenderable();
	obj.flags = 0;
	SceneObject* res = SC_AddObject(scene, &obj);

	base->obj = res;

	base->left->GetUserData().pointer = (uintptr_t)res;
	base->right->GetUserData().pointer = (uintptr_t)res;
	base->top->GetUserData().pointer = (uintptr_t)res;

	GM_GetGameManager()->background = res;

	return res;
}

SceneObject* CreateBallObject(Scene* scene, const glm::vec2& pos, const glm::vec2& velocity, float size)
{
	GameState* game = GetGameState();
	b2BodyDef body{};
	body.type = b2_dynamicBody;
	body.position = { pos.x, pos.y };
	body.angle = 0.0f;
	body.enabled = true;
	body.allowSleep = false;
	body.fixedRotation = false;
	body.gravityScale = 1.0f;
	b2Body* collBody = game->physics->world.CreateBody(&body);

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

	collBody->CreateFixture(&fixture);

	GameManager* m = (GameManager*)game->manager;

	const AtlasTexture::UVBound& bound = m->atlas->bounds[SPRITES::DISH_2];

	SceneObject obj;
	obj.entity = new Ball(pos);
	obj.body = collBody;
	obj.renderable = new TextureQuad(pos, {(size + PADDING), (size + PADDING) }, bound.start, bound.end, m->atlas->texture.uniform);
	obj.flags = 0;
	obj.renderable->layer = 1;
	SceneObject* res = SC_AddObject(scene, &obj);
	collBody->GetUserData().pointer = (uintptr_t)res;

	res->body->SetLinearVelocity({ velocity.x, velocity.y });
	collBody->SetAwake(true);
	
	((PeggleEntity*)res->entity)->obj = res;

	GM_GetGameManager()->ballList.push_back(res);
	return res;
}

SceneObject* CreatePegObject(Scene* scene, const glm::vec2& pos, float size, ENTITY_TYPE type)
{

	SPRITES pegSprites[ENTITY_TYPE::NUM_ENTITYS] = {
		SPRITES::DISH_2, SPRITES::DISH_2,
		SPRITES::PIZZA, SPRITES::BURGER,
	};

	GameState* game = GetGameState();

	b2BodyDef body{};
	body.type = b2_staticBody;
	body.position = { pos.x, pos.y };
	body.angle = 0.0f;
	body.enabled = true;
	body.allowSleep = true;
	body.fixedRotation = false;
	body.gravityScale = 1.0f;
	b2Body* collBody = game->physics->world.CreateBody(&body);

	b2CircleShape shape;
	shape.m_radius = size;

	b2FixtureDef fixture{};
	fixture.density = 1.0f;
	fixture.friction = 0.1f;
	fixture.isSensor = false;
	fixture.restitution = 0.1f;
	fixture.restitutionThreshold = 0.0f;
	fixture.shape = &shape;

	collBody->CreateFixture(&fixture);

	GameManager* m = (GameManager*)game->manager;

	const AtlasTexture::UVBound& bound = m->atlas->bounds[pegSprites[type]];

	SceneObject obj;
	obj.entity = new Peg(pos, type);
	obj.body = collBody;
	obj.renderable = new TextureQuad(pos, { (size + PADDING), (size + PADDING) }, bound.start, bound.end, m->atlas->texture.uniform);
	obj.flags = 0;
	obj.renderable->layer = 1;
	SceneObject* res = SC_AddObject(scene, &obj);
	collBody->GetUserData().pointer = (uintptr_t)res;
	((PeggleEntity*)res->entity)->obj = res;

	m->pegList.push_back(res);

	return res;
}

SceneObject* CreatePlayerObject(Scene* scene, const glm::vec2& pos, float size)
{
	SceneObject obj;
	Player* p = new Player;
	AnimatedQuad* anim = new AnimatedQuad(pos, { size, size }, GM_GetGameManager()->atlas, 0xFFFFFFFF, 1);
	obj.entity = p;
	obj.body = nullptr;
	obj.renderable = anim;
	obj.flags = 0;
	SceneObject* res = SC_AddObject(scene, &obj);
	
	anim->AddAnimRange(SPRITES::FOX_IDLE_0, SPRITES::FOX_IDLE_4 + 1);
	anim->AddAnimRange(SPRITES::FOX_LEAP_0, SPRITES::FOX_LEAP_10 + 1);
	anim->AddAnimRange(SPRITES::FOX_SHOCK_0, SPRITES::FOX_SHOCK_4 + 1);

	((Character*)res->entity)->obj = res;
	GM_GetGameManager()->player = res;

	return res;
}
SceneObject* CreateProjectileObject(Scene* scene, const glm::vec2& pos, float size)
{
	Projectile* proj = new Projectile();
	proj->flags = 0;
	proj->type = BASE_PROJECTILE;

	GameManager* m = GM_GetGameManager();
	AtlasTexture::UVBound& bound = m->atlas->bounds[DISH_2];

	SceneObject obj;
	obj.body = nullptr;
	obj.entity = proj;
	obj.flags = 0;
	obj.renderable = new TextureQuad(pos, { size, size }, bound.start, bound.end, m->atlas->texture.uniform, 0xFFFFFFFF, 1);

	SceneObject* res = SC_AddObject(scene, &obj);
	proj->obj = res;

	m->projList.push_back(res);

	return res;
}



void RemoveBaseObject()
{
	GameManager* m = GM_GetGameManager();
	GameState* state = GetGameState();
	if (m->background)
	{
		SC_RemoveObject(state->scene, m->background);
		m->background = nullptr;
	}
}
void RemoveBallObject(size_t idx)
{
	GameManager* m = GM_GetGameManager();
	if (idx < m->ballList.size())
	{
		GameState* state = GetGameState();
		SC_RemoveObject(state->scene, m->ballList.at(idx));
		m->ballList.erase(m->ballList.begin() + idx);
	}
}
void RemovePegObject(size_t idx)
{
	GameManager* m = GM_GetGameManager();
	if (idx < m->pegList.size())
	{
		GameState* state = GetGameState();
		SC_RemoveObject(state->scene, m->pegList.at(idx));
		m->pegList.erase(m->pegList.begin() + idx);
	}
}
void RemoveProjectileObject(size_t idx)
{
	GameManager* m = GM_GetGameManager();
	if (idx < m->projList.size())
	{
		GameState* state = GetGameState();
		SC_RemoveObject(state->scene, m->projList.at(idx));
		m->projList.erase(m->projList.begin() + idx);
	}
}
void RemovePlayerObject()
{
	GameManager* m = GM_GetGameManager();
	GameState* state = GetGameState();
	if (m->background)
	{
		SC_RemoveObject(state->scene, m->player);
		m->player = nullptr;
	}
}
void RemoveAllBalls()
{
	GameManager* m = GM_GetGameManager();
	GameState* state = GetGameState();
	for (uint32_t i = 0; i < m->ballList.size(); i++)
	{
		SC_RemoveObject(state->scene, m->ballList.at(i));
	}
	m->ballList.clear();
}
void RemoveAllPegs()
{
	GameManager* m = GM_GetGameManager();
	GameState* state = GetGameState();
	for (uint32_t i = 0; i < m->pegList.size(); i++)
	{
		SC_RemoveObject(state->scene, m->pegList.at(i));
	}
	m->pegList.clear();
}
void RemoveAllProjectiles()
{
	GameManager* m = GM_GetGameManager();
	GameState* state = GetGameState();
	for (uint32_t i = 0; i < m->projList.size(); i++)
	{
		SC_RemoveObject(state->scene, m->projList.at(i));
	}
	m->projList.clear();
}
void RemoveAllObjects()
{
	GameManager* m = GM_GetGameManager();
	GameState* state = GetGameState();
	SC_RemoveAll(state->scene);
	m->pegList.clear();
	m->ballList.clear();
	m->background = nullptr;
	m->player = nullptr;
}