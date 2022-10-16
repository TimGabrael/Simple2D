#include "GameManager.h"


void GameManager::RenderCallback(GameState* state)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, state->winWidth, state->winHeight);
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	uint32_t num = 0;
	SceneObject** obj = SC_GetAllSceneObjects(state->scene, &num);

	glm::mat4 temp(1.0f);
	RE_RenderScene(state->renderer, temp, obj, num);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_::ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::ShowDemoWindow(nullptr);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GameManager::Update(float dt)
{
}

void GameManager::PostUpdate(float dt)
{
	GameState* state = GetGameState();
	uint32_t numInScene = 0;
	SceneObject** objs = SC_GetAllSceneObjects(state->scene, &numInScene);
	for (uint32_t i = 0; i < numInScene; i++)
	{
		if (objs[i]->body)
		{
			auto pos = objs[i]->body->GetPosition();
			float angle = objs[i]->body->GetAngle();
			if (pos.x < -1.0f) pos.x = -1.0f;
			if (pos.y < -1.0f) pos.y = -1.0f;
			if (pos.x > 1.0f) pos.x = 1.0f;
			if (pos.y > 1.0f) pos.y = 1.0f;
			objs[i]->body->SetTransform(pos, angle);
		}
	}
}

void GameManager::OnWindowPositionChanged(int x, int y)
{
}

void GameManager::OnWindowResize(int w, int h)
{
}

void GameManager::OnKey(int key, int scancode, int action, int mods)
{
}

void GameManager::OnMouseButton(int button, int action, int mods)
{
}

void GameManager::OnMousePositionChanged(float x, float y, float dx, float dy)
{
}

GameManager* GM_CreateGameManager(GameState* state)
{
	GameManager* out = new GameManager;

	b2BodyDef bd;
	bd.type = b2_staticBody;
	bd.position = { 0.0f, 0.0f };
	bd.angle = b2_pi;
	b2World* world = new b2World({ 0.0f, -9.81f });

	return out;
}