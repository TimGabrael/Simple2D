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


		//verts.push_back({ {m->vpStart.x, m->vpStart.y }, {0.0f, 1.0f}, 0xFFFFFFFF });
		//verts.push_back({ {m->vpEnd.x, m->vpStart.y }, {1.0f, 1.0f}, 0xFFFFFFFF });
		//verts.push_back({ {m->vpEnd.x, m->vpEnd.y }, {1.0f, 0.0f}, 0xFFFFFFFF });
		//verts.push_back({ {m->vpStart.x, m->vpEnd.y }, {0.0f, 0.0f}, 0xFFFFFFFF });
		//
		//inds.push_back(cur);
		//inds.push_back(cur + 1);
		//inds.push_back(cur + 2);
		//inds.push_back(cur + 2);
		//inds.push_back(cur + 3);
		//inds.push_back(cur);


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

Peg::Peg(const glm::vec2& pos)
{
	this->pos = pos;
}

ENTITY_TYPE Peg::GetType() const
{
	return ENTITY_TYPE::STANDARD_PEG;
}

void Peg::OnCollideWithBall(SceneObject* ball, b2Fixture* fixture, const glm::vec2& normal)
{
	ball->body->ApplyForceToCenter({ normal.x * 0.1f, normal.y * 0.1f }, true);
}



std::vector<glm::vec2> SimulateBall(const glm::vec2& pos, const glm::vec2& velocity, float size, float simulateDuration)
{
	GameState* state = GetGameState();
	GameManager* m = (GameManager*)state->manager;
	if (m->ballList.size() > 0) return {};

	std::vector<glm::vec2> accumulated = {pos};

	SceneObject* obj = CreateBallObject(state->scene, pos, velocity, size);
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
	return accumulated;
}


SceneObject* CreateBaseObject(Scene* scene)
{
	GameState* game = GetGameState();

	Base* base = new Base();
	{
		b2BodyDef body{};
		body.type = b2_staticBody;
		body.position = { -2.0f, 0.0f };
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


		body.position = { 2.0f, 0.0f };
		b2Body* collRight = game->physics->world.CreateBody(&body);
		collRight->CreateFixture(&fixture);
		base->right = collRight;


		body.position = { 0.0f, 2.0f };
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

	base->left->GetUserData().pointer = (uintptr_t)res;
	base->right->GetUserData().pointer = (uintptr_t)res;
	base->top->GetUserData().pointer = (uintptr_t)res;

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

	GM_GetGameManager()->ballList.push_back(res);
	return res;
}

SceneObject* CreatePegObject(Scene* scene, const glm::vec2& pos, float size)
{
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

	const AtlasTexture::UVBound& bound = m->atlas->bounds[SPRITES::PIZZA];

	SceneObject obj;
	obj.entity = new Peg(pos);
	obj.body = collBody;
	obj.renderable = new TextureQuad(pos, { (size + PADDING), (size + PADDING) }, bound.start, bound.end, m->atlas->texture.uniform);
	obj.flags = 0;
	obj.renderable->layer = 1;
	SceneObject* res = SC_AddObject(scene, &obj);
	collBody->GetUserData().pointer = (uintptr_t)res;

	return res;
}