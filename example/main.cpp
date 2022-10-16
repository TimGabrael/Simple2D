#include "GameManager.h"

int main()
{
	GameState* game = CreateGameState("Example2D", 1600, 900);
	GameManager* manager = GM_CreateGameManager(game);
	game->manager = manager;


	{
		b2BodyDef body{};
		body.type = b2_dynamicBody;
		body.position = { 0.10f, 0.10f };
		body.angle = 0.0f;
		body.enabled = true;
		body.allowSleep = true;
		body.fixedRotation = false;
		body.gravityScale = 1.0f;
		b2Body* collBody = game->physics->world.CreateBody(&body);

		b2PolygonShape shape;
		shape.SetAsBox(0.1f, 0.1f);

		b2FixtureDef fixture{};
		fixture.density = 1.0f;
		fixture.friction = 0.0f;
		fixture.isSensor = false;
		fixture.restitution = 0.1f;
		fixture.restitutionThreshold = 2.0f;
		fixture.shape = &shape;

		collBody->CreateFixture(&fixture);

		SceneObject obj;
		obj.entity = nullptr;
		obj.flags = 0;
		obj.body = collBody;
		obj.renderable = new TextureQuad({ 0.10f, 0.10f }, { 0.10f, 0.10f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, 0xFFFF00FF);
		SC_AddObject(game->scene, &obj);
	}
	{
		b2BodyDef body{};
		body.type = b2_staticBody;
		body.position = { 0.0f, -0.80f };
		body.angle = 0.1f;
		body.enabled = true;
		body.allowSleep = true;
		body.fixedRotation = false;
		body.gravityScale = 1.0f;
		b2Body* collBody = game->physics->world.CreateBody(&body);

		b2PolygonShape shape;
		shape.SetAsBox(2.f, 0.05f);

		b2FixtureDef fixture{};
		fixture.density = 1.0f;
		fixture.friction = 0.0f;
		fixture.isSensor = false;
		fixture.restitution = 0.1f;
		fixture.restitutionThreshold = 2.0f;
		fixture.shape = &shape;

		collBody->CreateFixture(&fixture);

		TextureQuad* quad = new TextureQuad({ 0.0f, -0.80f }, { 2.f, 0.05f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }, 0xFFFF0000);
		quad->angle = 0.1f;
		SceneObject obj;
		obj.entity = nullptr;
		obj.flags = 0;
		obj.body = collBody;
		obj.renderable = quad;
		SC_AddObject(game->scene, &obj);
	}



	UpateGameState();
	return 0;
}