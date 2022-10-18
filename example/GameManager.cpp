#include "GameManager.h"
#include "Entitys.h"
#include <random>
#include <string>

float GM_GetRandomFloat(float start, float end)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution dist(start, end);
	return dist(mt);
}


void GameManager::RenderCallback(GameState* state, float dt)
{
	if (attackCycleState == BALLS_FALLING)
	{
		if (ballList.empty())
		{
			attackCycleState = ATTACK_CYCLE_STATES::PLAYER_TURN;
			player->SetAnimation(Player::ATTACK);

			AnimatedQuad& q = player->quad;
			projList.push_back(new Projectile(q.pos + glm::vec2(0.1f, -0.1f), 0.05f, SPRITES::DISH_2));
			waitTimer = 0.0f;
		}
	}
	else if (attackCycleState == PLAYER_TURN)
	{
		if (projList.empty())
		{
			if (waitTimer < 0.04f) waitTimer += dt;
			else
			{
				waitTimer = 0.0f;
				attackCycleState = ATTACK_CYCLE_STATES::ENEMY_TURN;
				curEnemyInTurn = 0;
			}
		}
	}
	else if (attackCycleState == ATTACK_CYCLE_STATES::ENEMY_TURN)
	{
		if (curEnemyInTurn < enemyList.size() && curEnemyInTurn >= 0)
		{
			Character* curEnemy = enemyList.at(curEnemyInTurn);
			if (projList.empty() && curEnemy->activeAnimation == Character::IDLE)
			{
				if (waitTimer < 0.04f) waitTimer += dt;
				else 
				{
					waitTimer = 0.0f;
					curEnemyInTurn++;
				}
			}
		}
		else
		{
			curEnemyInTurn = 0;
			waitTimer = 0.0f;
			attackCycleState = ATTACK_CYCLE_STATES::MOVE_TURN;
		}
	}
	else if (attackCycleState == ATTACK_CYCLE_STATES::MOVE_TURN)
	{
		if (curEnemyInTurn < enemyList.size() && curEnemyInTurn >= 0)
		{
			Character* curEnemy = enemyList.at(curEnemyInTurn);
			const float desiredPos = background->entXStart + (curEnemyInTurn + 1) * background->xSteps;		// +1 as player is on 0
			if (curEnemy->quad.pos.x <= desiredPos)
			{
				curEnemy->SetAnimation(Character::IDLE);
				//if (waitTimer < 0.04f) waitTimer += dt;
				//else
				{
					waitTimer = 0.0f;
					curEnemyInTurn++;
				}
			}
			else
			{
				curEnemy->SetAnimation(Character::MOVE);
				curEnemy->quad.pos.x = glm::max(desiredPos, curEnemy->quad.pos.x - 1.0f * dt);
			}
		}
		else
		{
			curEnemyInTurn = 0;
			waitTimer = 0.0f;
			attackCycleState = ATTACK_CYCLE_STATES::WAIT_FOR_INPUT;
		}
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, state->winWidth, state->winHeight);
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RE_RenderScene(state->renderer, viewProj, state->scene);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	

	ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_::ImGuiDockNodeFlags_PassthruCentralNode);

	//ImGui::ShowDemoWindow(nullptr);

	//ImGui::GetForegroundDrawList()->AddImage((ImTextureID)atlas->texture.uniform, { 100.0f, 100.0f }, { 1000.0f, 1000.0f });

	if (background)
	{
		const glm::vec2 vpSz = vpEnd - vpStart;
		glm::vec2 relMouse = { state->mouseX / (float)state->winWidth, state->mouseY / (float)state->winHeight };
		relMouse = vpStart + vpSz * relMouse;
		relMouse.y = -relMouse.y;
		relMouse = glm::normalize(relMouse - background->startPos);

		targetDir = relMouse;


		std::vector<glm::vec2> list = SimulateBall(background->startPos, targetDir * startVelocity, 0.05f, 100.0f);
		if (list.size() > 0)
		{
			ImVec2* imList = new ImVec2[list.size()];
			for (uint32_t i = 0; i < list.size(); i++)
			{
				imList[i].x = (list.at(i).x - vpStart.x) / (vpEnd.x - vpStart.x) * state->winWidth;
				imList[i].y = (list.at(i).y - vpEnd.y) / (vpStart.y - vpEnd.y) * state->winHeight;
			}
			ImGui::GetBackgroundDrawList()->AddPolyline(imList, list.size(), 0x80FFFFFF, 0, 2.0f);
			delete[] imList;
		}

		const glm::vec2 winStart = {((float)state->winWidth / vpSz.x) * (background->endBound.x - vpStart.x), ((float)state->winHeight / vpSz.y) * (vpEnd.y - background->endBound.y) };
		const glm::vec2 winSize = { ((float)state->winWidth / vpSz.x) * (vpEnd.x - background->endBound.x + 0.01f), ((float)state->winHeight / vpSz.y) * (background->endBound.y - vpStart.y + 0.01f) };
		

		ImGui::SetNextWindowDockID(0);

		ImGui::SetNextWindowPos({ winStart.x, winStart.y });
		ImGui::SetNextWindowSize({winSize.x, winSize.y});

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_WindowBg, 0xFF404060);


		ImGui::Begin("DATA", 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
		ImGui::Text("DAMAGE: %s", std::to_string(accumulatedDamage).c_str());
		ImGui::End();

		ImGui::PopStyleColor();
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
				PeggleEntity* uA = (PeggleEntity*)bA->GetUserData().pointer;
				PeggleEntity* uB = (PeggleEntity*)bB->GetUserData().pointer;
				if (uA && uB)
				{
					b2WorldManifold normal;
					list->GetWorldManifold(&normal);
					glm::vec2 n = { normal.normal.x, normal.normal.y };
					if (uA->GetType() == ENTITY_TYPE::BALL && uB->GetType() == ENTITY_TYPE::BALL)
					{
						uA->OnCollideWithBall((Ball*)uB, fixA, n);
						uB->OnCollideWithBall((Ball*)uA, fixB, -n);
					}
					else if (uA->GetType() == ENTITY_TYPE::BALL && uB->GetType() != ENTITY_TYPE::BALL)
					{
						uB->OnCollideWithBall((Ball*)uA, fixB, -n);
					}
					else if (uA->GetType() != ENTITY_TYPE::BALL && uB->GetType() == ENTITY_TYPE::BALL)
					{
						uA->OnCollideWithBall((Ball*)uB, fixA, n);
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
		// FREE EVERYTHING
		// FILL EVERYTHING
		//GM_FillScene();
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_P)
	{
		GameManager* m = GM_GetGameManager();
		if (m->player)
		{
			m->player->SetAnimation(Player::HURT);
		}
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_K)
	{
		GameManager* m = GM_GetGameManager();
		if (m->player)
		{
			m->player->SetAnimation(Player::ATTACK);
		}
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
			if (attackCycleState == WAIT_FOR_INPUT)
			{
				m->accumulatedDamage = 0;
				Base* b = m->background;
				m->ballList.push_back(new Ball(b->startPos, m->targetDir * m->startVelocity, 0.05f));
				attackCycleState = BALLS_FALLING;
			}
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

void GM_AddParticle(const glm::vec2& pos, const glm::vec2& vel, const glm::vec2& sizeBegin, const glm::vec2& sizeEnd, uint32_t colBegin, uint32_t colEnd, SPRITES sprite, float rotationBegin, float rotationEnd, float lifeTime)
{
	if (GM_GetGameManager()->particleHandler)
	{
		ParticlesBase& b = GM_GetGameManager()->particleHandler->base;
		b.AddParticle(pos, vel, sizeBegin, sizeEnd, colBegin, colEnd, sprite, rotationBegin, rotationEnd, lifeTime);
	}
}

void GM_FillScene()
{
	GameState* game = GetGameState();
	GameManager* m = (GameManager*)game->manager;
	
	m->background = new Base(glm::vec2(0.0f, 1.1f), glm::vec2(-1.5f), glm::vec2(1.5f));
	m->particleHandler = new ParticleHandlerEntity;

	m->player = new Player({ m->background->entXStart, m->background->entYStart }, 0.2f);
	
	for (uint32_t i = 1; i < m->background->numInRow; i++)
	{
		Character* enemy = CreateEnemy({ m->background->entXStart + i * m->background->xSteps, m->background->entYStart }, 0.2f, CHARACTER_TYPES::SLIME);
		if (enemy)
		{
			m->enemyList.push_back(enemy);
		}
	}

	GM_CreateFieldFromCharacters(m->background,
		"###   ###   ###\n   ###   ###   \n###   ###   ###\n   ###   ###   \n###   ###   ###\n   ###   ###   \n###   ###   ###\n   ###   ###   \n###   ###   ###\n   ###   ###   \n");
}

void GM_CreateFieldFromCharacters(Base* b, const char* field)
{
	GameState* state = GetGameState();
	GameManager* m = (GameManager*)state->manager;
	const glm::vec2 playSize = b->endBound - b->startBound - glm::vec2(0.1f, 0.5f);

	float curY = b->endBound.y - 0.8f;

	const size_t fieldLen = strnlen(field, 100000);
	uint32_t lineCount = 1;
	for (uint32_t i = 0; i < fieldLen-1; i++)
	{
		if (field[i] == '\n') lineCount++;
	}

	const float stepY = playSize.y / (float)lineCount;


	size_t first = 0;
	while (true)
	{
		size_t lineEnd = -1;
		for (size_t j = first; j < fieldLen; j++)
		{
			if (field[j] == '\n') {
				lineEnd = j;
				break;
			}
		}
		if (lineEnd == -1) lineEnd = fieldLen;

		if (first == lineEnd) break;

		const uint32_t steps = lineEnd - first;

		float stepX = playSize.x / (float)steps;

		for (size_t i = 0; i < steps; i++)
		{
			if (field[i + first] == ' ') continue;
			else
			{
				glm::vec2 pos = {b->startBound.x + i * stepX + 0.1f, curY};
				m->pegList.push_back(new Peg(pos, 0.05f, GM_GenerateRandomPegType()));
			}
		}
		curY -= stepY;
		first = lineEnd + 1;
	}
}

ENTITY_TYPE GM_GenerateRandomPegType()
{
	const float rd = GM_GetRandomFloat(0.0f, 1.0f);
	if (rd < 0.95f) return ENTITY_TYPE::STANDARD_PEG;
	else return ENTITY_TYPE::REFRESH_PEG;
}

void GM_PlaySound(SOUNDS sound, float volume)
{
	GameState* s = GetGameState();
	GameManager* m = GM_GetGameManager();
	AU_PlayAudio(s->audio, m->audioFiles.at(sound), volume);
}