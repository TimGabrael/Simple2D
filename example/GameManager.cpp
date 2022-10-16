#include "GameManager.h"
#include "Entitys.h"
#include <random>

float GetRandomFloat(float start, float end)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution dist(start, end);
	return dist(mt);
}


void GameManager::RenderCallback(GameState* state)
{
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, state->winWidth, state->winHeight);
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	uint32_t num = 0;
	SceneObject** obj = SC_GetAllSceneObjects(state->scene, &num);

	RE_RenderScene(state->renderer, viewProj, obj, num);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	

	ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_::ImGuiDockNodeFlags_PassthruCentralNode);

	//ImGui::ShowDemoWindow(nullptr);

	//ImGui::GetForegroundDrawList()->AddImage((ImTextureID)atlas->texture.uniform, { 100.0f, 100.0f }, { 1000.0f, 1000.0f });

	if (background)
	{
		Base* b = (Base*)background->entity;
		glm::vec2 relMouse = { state->mouseX / (float)state->winWidth, state->mouseY / (float)state->winHeight };
		relMouse = vpStart + (vpEnd - vpStart) * relMouse;
		relMouse.y = -relMouse.y;
		relMouse = glm::normalize(relMouse - b->startPos);

		targetDir = relMouse;


		std::vector<glm::vec2> list = SimulateBall(b->startPos, targetDir * startVelocity, 0.05f, 50.0f);
		if (list.size() > 0)
		{
			ImVec2* imList = new ImVec2[list.size()];
			for (uint32_t i = 0; i < list.size(); i++)
			{
				imList[i].x = (list.at(i).x - vpStart.x) / (vpEnd.x - vpStart.x) * state->winWidth;
				imList[i].y = (list.at(i).y - vpEnd.y) / (vpStart.y - vpEnd.y) * state->winHeight;

			}
			ImGui::GetForegroundDrawList()->AddPolyline(imList, list.size(), 0x80FFFFFF, 0, 2.0f);
			delete[] imList;
		}
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GameManager::Update(float dt)
{

}

void GameManager::PostUpdate(float dt)
{
	GameState* state = GetGameState();
	b2Contact* list = state->physics->world.GetContactList();
	while (list)
	{
		if (list->IsTouching())
		{
			b2Fixture* fixA = list->GetFixtureA();
			b2Fixture* fixB = list->GetFixtureB();
			b2Body* bA = fixA->GetBody();
			b2Body* bB = fixB->GetBody();
			if (bA && bB)
			{
				SceneObject* uA = (SceneObject*)bA->GetUserData().pointer;
				SceneObject* uB = (SceneObject*)bB->GetUserData().pointer;
				if (uA && uB && uA->entity && uB->entity)
				{
					PeggleEntity* eA = (PeggleEntity*)uA->entity;
					PeggleEntity* eB = (PeggleEntity*)uB->entity;

					b2WorldManifold normal;
					list->GetWorldManifold(&normal);
					glm::vec2 n = { normal.normal.x, normal.normal.y };
					if (eA->GetType() == ENTITY_TYPE::BALL && eB->GetType() == ENTITY_TYPE::BALL)
					{
						eA->OnCollideWithBall(uB, fixA, n);
						eB->OnCollideWithBall(uA, fixB, -n);
					}
					else if (eA->GetType() == ENTITY_TYPE::BALL && eB->GetType() != ENTITY_TYPE::BALL)
					{
						eB->OnCollideWithBall(uA, fixB, -n);
					}
					else if (eA->GetType() != ENTITY_TYPE::BALL && eB->GetType() == ENTITY_TYPE::BALL)
					{
						eA->OnCollideWithBall(uB, fixA, n);
					}

				}
			}
		}
		list = list->GetNext();
	}
}

void GameManager::OnWindowPositionChanged(int x, int y)
{
}

void GameManager::OnWindowResize(int w, int h)
{
	GameState* state = GetGameState();
	vpStart = { -2.0f * state->aspectRatio, -2.0f };
	vpEnd = { 2.0f * state->aspectRatio, 2.0f };
	viewProj = glm::orthoRH(vpStart.x, vpEnd.x, vpStart.y, vpEnd.y, 0.0f, 1.0f);
}

void GameManager::OnKey(int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_R)
	{
		GameState* state = GetGameState();
		GameManager* m = (GameManager*)state->manager;
		SC_RemoveAll(state->scene);
		m->ballList.clear();
		FillScene();
	}
}

void GameManager::OnMouseButton(int button, int action, int mods)
{
	GameState* game = GetGameState();
	
	if (action == GLFW_PRESS)
	{
		GameManager* m = GM_GetGameManager();
		if (button == GLFW_MOUSE_BUTTON_LEFT && m->background)
		{
			Base* b = (Base*)m->background->entity;
			SceneObject* obj = CreateBallObject(game->scene, b->startPos, { m->targetDir * m->startVelocity }, 0.05f);
		}
	}
}

void GameManager::OnMousePositionChanged(float x, float y, float dx, float dy)
{
}

GameManager* GM_CreateGameManager(GameState* state)
{
	GameManager* out = new GameManager;

	out->vpStart = { -2.0f * state->aspectRatio, -2.0f };
	out->vpEnd = { 2.0f * state->aspectRatio, 2.0f };
	out->viewProj = glm::orthoRH(out->vpStart.x, out->vpEnd.x, out->vpStart.y, out->vpEnd.y, 0.0f, 1.0f);

	out->startVelocity = 8.0f;
	
	return out;
}


GameManager* GM_GetGameManager()
{
	return (GameManager*)GetGameState()->manager;
}


void FillScene()
{
	GameState* game = GetGameState();
	SceneObject* base = CreateBaseObject(game->scene);
	for (uint32_t i = 0; i < 300; i++)
	{
		SceneObject* peg = CreatePegObject(game->scene, { GetRandomFloat(-1.4f, 1.4f), GetRandomFloat(-1.5f, 1.0f) }, 0.05f);
	}
}