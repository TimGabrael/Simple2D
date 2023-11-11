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

void GameManager::DrawUi(GameState* state)
{
	if (background)
	{
		const glm::vec2 vpSz = vpEnd - vpStart;
		const glm::vec2 vpToScreen = { (float)state->winWidth / vpSz.x, (float)state->winHeight / -vpSz.y };
		ImDrawList* drawList = ImGui::GetBackgroundDrawList();

		if (!overlayMenuActive)
		{
			std::vector<glm::vec2> list = SimulateBall(background->startPos, targetDir * startVelocity, 0.05f, 1.0f);
			if (list.size() > 0)
			{
				ImVec2* imList = new ImVec2[list.size()];
				for (uint32_t i = 0; i < list.size(); i++)
				{
					imList[i].x = (list.at(i).x - vpStart.x) * vpToScreen.x;
					imList[i].y = (list.at(i).y - vpEnd.y) * vpToScreen.y;
				}
				drawList->AddPolyline(imList, (int)list.size(), 0x80FFFFFF, 0, 2.0f);
				delete[] imList;
			}

			if (state->tickMultiplier == 0.0f)
			{
				drawList->AddText({ 0.0f, 50.0f }, 0xFF0000FF, "PAUSED: PRESS P TO UNPAUSE");
			}
		}
		// LEFT SIDE
		{
			const glm::vec2 winStart = { 0.0f, vpToScreen.y * (background->endBound.y - vpEnd.y) };
			const glm::vec2 winSize = { vpToScreen.x * (background->startBound.x - vpStart.x) + 1.0f, vpToScreen.y * (vpStart.y - background->endBound.y) + 1.0f};

			ImGui::SetNextWindowPos({ winStart.x, winStart.y });
			ImGui::SetNextWindowSize({ winSize.x, winSize.y });

			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_WindowBg, 0x0);

			ImGui::Begin("PLAYER_INFO", 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
			ImGui::Spacing();ImGui::Spacing();ImGui::Spacing();ImGui::Spacing();
			if (player)
			{
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_PlotHistogram, 0xFF00FF00);
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_FrameBg, 0xFF0000FF);
				ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Text, 0xFF000000);

				float hpFraction = (float)player->health / (float)player->maxHealth;
				std::string hpStr = "Health: " + std::to_string(player->health);
				ImGui::ProgressBar(hpFraction, ImVec2(-FLT_MIN, 0.0f), hpStr.c_str());

				ImGui::PopStyleColor(3);
			}
			ImGui::End();

			ImGui::PopStyleColor();
		}


		// DRAW ENEMY HEALTH BARS
		for (uint32_t i = 0; i < enemyList.size(); i++)
		{
			const glm::vec2& pos = enemyList.at(i)->quad.pos;
			const glm::vec2& sz = enemyList.at(i)->quad.halfSize;

			const float ratio = (float)enemyList.at(i)->health / (float)enemyList.at(i)->maxHealth;

			ImVec2 start = { (pos.x - sz.x - vpStart.x + 0.05f) * vpToScreen.x, vpToScreen.y * (pos.y - vpEnd.y + sz.y) };
			ImVec2 end = { (pos.x + sz.x - vpStart.x - 0.05f) * vpToScreen.x, vpToScreen.y * (pos.y - vpEnd.y + sz.y) + 10.0f};


			drawList->AddRectFilled(start, end, 0xFF0000FF);

			end.x = (end.x - start.x) * ratio + start.x;

			drawList->AddRectFilled(start, end, 0xFF00FF00);
		}


		// RIGHT SIDE
		{
			const glm::vec2 winStart = { vpToScreen.x * (background->endBound.x - vpStart.x), vpToScreen.y * (background->endBound.y - vpEnd.y) };
			const glm::vec2 winSize = { vpToScreen.x * (vpEnd.x - background->endBound.x) + 1.0f, vpToScreen.y * (vpStart.y - background->endBound.y) + 1.0f};



			ImGui::SetNextWindowPos({ winStart.x, winStart.y });
			ImGui::SetNextWindowSize({ winSize.x, winSize.y });

			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_WindowBg, 0xFF404060);


			ImGui::Begin("DATA", 0, ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar);
			ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();
			ImGui::Text("DAMAGE: %d", (int)stats.accumulatedDamage);
			ImGui::End();

			ImGui::PopStyleColor();
		}
	}
}
void GameManager::UpdateBattleState(GameState* state, float dt)
{
	static bool switchedEnemyInTurn = false;
	if (attackCycleState == BALLS_FALLING)
	{
		if (ballList.empty())
		{
			attackCycleState = ATTACK_CYCLE_STATES::PLAYER_TURN;
			player->SetAnimation(Player::ATTACK);

			AnimatedQuad& q = player->quad;
			projList.push_back(new Projectile(q.pos + glm::vec2(0.0f, -0.1f), 0.1f, SPRITES::DISH_2));
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
				bool allIdle = true;
				for (Character* enemy : enemyList)
				{
					if (enemy->activeAnimation != Character::IDLE)
					{
						allIdle = false;
					}
				}
				if (allIdle)
				{
					waitTimer = 0.0f;
					attackCycleState = ATTACK_CYCLE_STATES::ENEMY_TURN;
					curEnemyInTurn = 0;
					switchedEnemyInTurn = true;
					enemyActionList = enemyList;
				}
			}
		}
	}
	else if (attackCycleState == ATTACK_CYCLE_STATES::ENEMY_TURN)
	{

		if (curEnemyInTurn < enemyActionList.size() && curEnemyInTurn >= 0)
		{
			Character* curEnemy = enemyActionList.at(curEnemyInTurn);
			bool found = false;
			for (uint32_t i = 0; i < enemyList.size(); i++)
			{
				if (curEnemy == enemyList.at(i))
				{
					found = true;
				}
			}
			if (found)
			{
				if (switchedEnemyInTurn)
				{
					curEnemy->BeginAction();
					switchedEnemyInTurn = false;
				}
				if (curEnemy->PerformAction(dt))
				{
					waitTimer = 0.0f;
					curEnemyInTurn++;
					switchedEnemyInTurn = true;
				}
			}
			else
			{
				waitTimer = 0.0f;
				curEnemyInTurn++;
				switchedEnemyInTurn = true;
			}
		}
		else
		{
			curEnemyInTurn = 0;
			waitTimer = 0.0f;
			attackCycleState = ATTACK_CYCLE_STATES::WAIT_FOR_INPUT;
		}
	}

	// SET TARGET DIRECTION
	if (background)
	{
		const glm::vec2 vpSz = vpEnd - vpStart;
		glm::vec2 relMouse = { state->mouseX / (float)state->winWidth, state->mouseY / (float)state->winHeight };
		relMouse = vpStart + vpSz * relMouse;
		relMouse.y = -relMouse.y;
		relMouse = glm::normalize(relMouse - background->startPos);
		targetDir = relMouse;
	}
	DrawUi(state);
}

void GameManager::DrawMenu(GameState* state)
{
	
	ImGui::SetNextWindowPos({ 0.0f,0.0f });
	ImGui::SetNextWindowSize({ (float)state->winWidth, (float)state->winHeight });
	
	ImGui::Begin("MAIN MENU", nullptr,  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);


	const float btnWidth = (float)state->winWidth / 4.0f;
	const float btnHeight = (float)state->winHeight / 10.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, btnHeight / 2.0f });

	ImVec2 btnSize = ImVec2(btnWidth, btnHeight);
	ImGui::SetCursorPos({ ((float)state->winWidth - btnWidth) / 2.0f, ImGui::GetCursorPosY() + btnHeight * 4.0f });
	if (ImGui::Button("START", btnSize))
	{
		this->activeState = GAME_STATE_BATTLE;
		GM_FillScene();
	}
	ImGui::SetCursorPos({ ((float)state->winWidth - btnWidth) / 2.0f, ImGui::GetCursorPosY() });
	if (ImGui::Button("SETTINGS", btnSize))
	{
	}
	ImGui::SetCursorPos({ ((float)state->winWidth - btnWidth) / 2.0f, ImGui::GetCursorPosY() });
	if (ImGui::Button("CREDITS", btnSize))
	{

	}
	ImGui::SetCursorPos({ ((float)state->winWidth - btnWidth) / 2.0f, ImGui::GetCursorPosY() });
	if (ImGui::Button("EXIT", btnSize))
	{
		glfwSetWindowShouldClose(state->window, 1);
	}

	ImGui::PopStyleVar(1);

	ImGui::End();
}
void GameManager::DrawOverlayMenu(GameState* state)
{
	ImGui::SetNextWindowPos({ 0.0f,0.0f });
	ImGui::SetNextWindowSize({ (float)state->winWidth, (float)state->winHeight });

	ImGui::Begin("MENU", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

	const float btnWidth = (float)state->winWidth / 4.0f;
	const float btnHeight = (float)state->winHeight / 10.0f;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0.0f, btnHeight / 2.0f });

	ImVec2 btnSize = ImVec2(btnWidth, btnHeight);
	ImGui::SetCursorPos({ ((float)state->winWidth - btnWidth) / 2.0f, ImGui::GetCursorPosY() + btnHeight * 4.0f });
	if (ImGui::Button("CONTINUE", btnSize))
	{
		state->tickMultiplier = 1.0f;
		overlayMenuActive = false;
	}
	ImGui::SetCursorPos({ ((float)state->winWidth - btnWidth) / 2.0f, ImGui::GetCursorPosY() });
	if (ImGui::Button("SETTINGS", btnSize))
	{
	}
	ImGui::SetCursorPos({ ((float)state->winWidth - btnWidth) / 2.0f, ImGui::GetCursorPosY() });
	if (ImGui::Button("CREDITS", btnSize))
	{

	}
	ImGui::SetCursorPos({ ((float)state->winWidth - btnWidth) / 2.0f, ImGui::GetCursorPosY() });
	if (ImGui::Button("EXIT", btnSize))
	{
		glfwSetWindowShouldClose(state->window, 1);
	}

	ImGui::PopStyleVar(1);

	ImGui::End();
}

void GameManager::RenderCallback(GameState* state, float dt)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::DockSpaceOverViewport(nullptr, ImGuiDockNodeFlags_::ImGuiDockNodeFlags_PassthruCentralNode);


	if (this->activeState == GAME_STATE_BATTLE)	UpdateBattleState(state, dt);
	else if (activeState == GAME_STATE_MAIN_MENU) DrawMenu(state);

	if (this->overlayMenuActive) DrawOverlayMenu(state);
	
	glBindFramebuffer(GL_FRAMEBUFFER, ppData.intermediateFbo);
	glViewport(0, 0, state->winWidth, state->winHeight);
	glClearColor(0.4f, 0.4f, 0.4f, 1.0f);
	glClearDepthf(1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	RE_RenderScene(state->renderer, viewProj, state->scene);


	RE_RenderPostProcessingBloom(state->renderer, &ppData, ppData.intermediateTexture, state->winWidth, state->winHeight, 0, state->winWidth, state->winHeight);


	//ImGui::ShowDemoWindow(nullptr);
	//ImGui::GetForegroundDrawList()->AddImage((ImTextureID)atlas->texture.uniform, { 100.0f, 100.0f }, { 1000.0f, 1000.0f });

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GameManager::Update(float dt)
{
	
}

void GameManager::PostUpdate(float dt)
{
	std::vector<b2Contact*> contactList;
	{
		GameState* state = GetGameState();
		b2Contact* list = state->physics->world.GetContactList();
		while (list)
		{
			if (list->IsTouching())
			{
				contactList.push_back(list);
			}
			list = list->GetNext();
		}
	}
	for (b2Contact* contact : contactList)
	{
		b2Fixture* fixA = contact->GetFixtureA();
		b2Fixture* fixB = contact->GetFixtureB();
		b2Body* bA = fixA->GetBody();
		b2Body* bB = fixB->GetBody();
		if (bA && bB)
		{
			PeggleEntity* uA = (PeggleEntity*)bA->GetUserData().pointer;
			PeggleEntity* uB = (PeggleEntity*)bB->GetUserData().pointer;
			if (uA && uB)
			{
				b2WorldManifold normal;
				contact->GetWorldManifold(&normal);
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

	RE_CleanUpPostProcessingRenderData(&ppData);
	RE_CreatePostProcessingRenderData(&ppData, state->winWidth, state->winHeight);
}

void GameManager::OnKey(int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS && key == GLFW_KEY_R)
	{
		GM_ClearScene();
		GM_FillScene();
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_P)
	{
		GameState* s = GetGameState();
		if (!overlayMenuActive && activeState != GAME_STATE_MAIN_MENU)
		{
			if (s->tickMultiplier == 0.0f) s->tickMultiplier = 1.0f;
			else s->tickMultiplier = 0.0f;
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
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
	{
		GameManager* m = GM_GetGameManager();
		if (m->activeState != GAME_STATE_MAIN_MENU)
		{
			if (m->overlayMenuActive)
			{
				m->overlayMenuActive = false;
				GameState* s = GetGameState();
				s->tickMultiplier = 1.0f;
			}
			else
			{
				m->overlayMenuActive = true;
				GameState* s = GetGameState();
				s->tickMultiplier = 0.0f;
			}
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
				m->stats.accumulatedDamage = 0.0f;
				m->stats.critMultiplier = 1.0f;
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
	out->activeState = GAME_STATE_MAIN_MENU;
	
	RE_CreatePostProcessingRenderData(&out->ppData, state->winWidth, state->winHeight);

	return out;
}


GameManager* GM_GetGameManager()
{
	return (GameManager*)GetGameState()->manager;
}

void GM_AddParticle(const glm::vec2& pos, const glm::vec2& vel, const glm::vec2& sizeBegin, const glm::vec2& sizeEnd, const glm::vec4& colBegin, const glm::vec4& colEnd, SPRITES sprite, float rotationBegin, float rotationEnd, float lifeTime)
{
	if (GM_GetGameManager()->particleHandler)
	{
		ParticlesBase& b = GM_GetGameManager()->particleHandler->base;
		b.AddParticle(pos, vel, sizeBegin, sizeEnd, colBegin, colEnd, sprite, rotationBegin, rotationEnd, lifeTime);
	}
}
void GM_AddTextParticle(const char* text, glm::vec2& center, const glm::vec2& vel, const glm::vec2& velVariation, float sizeBegin, float sizeEnd, const glm::vec4& colBegin, const glm::vec4& colEnd, float lifeTime)
{
	GameState* s = GetGameState();
	GameManager* m = GM_GetGameManager();
	if (m->particleHandler)
	{
		ParticlesBase& b = GM_GetGameManager()->particleHandler->base;

		float pixelSize = 0.0f;
		const int len = (int)strnlen(text, 1000);
		const float spaceAdvance = roundf(m->metrics->size * 2.0f / 4.0f * sizeBegin);

		// CALCULATE PIXEL SIZE OF THE TEXT
		for (int i = 0; i < len; i++)
		{
			char c = text[i];
			if (c == ' ') { pixelSize += spaceAdvance; continue; }
			uint32_t idx = c - m->metrics->firstCharacter;
			if (idx < m->metrics->numGlyphs)
			{
				Glyph& g = m->metrics->glyphs[idx];
				pixelSize += g.advance * sizeBegin;
			}
		}
		
		const float pixelToVP = (m->vpEnd.x - m->vpStart.x) / s->winWidth;

		float vpSizeX = pixelSize * pixelToVP;

		glm::vec2 curPos = glm::vec2(center.x - vpSizeX / 2.0f, center.y);

		// ADD THE GLYPHS TO THE PARTICLE SYSTEM
		for (int i = 0; i < len; i++)
		{
			char c = text[i];
			if (c == ' ') { curPos.x += spaceAdvance * pixelToVP; continue; }
			uint32_t idx = c - m->metrics->firstCharacter;
			if (idx < m->metrics->numGlyphs)
			{
				Glyph& g = m->metrics->glyphs[idx];
				curPos.x += g.advance * pixelToVP * sizeBegin;
				
				glm::vec2 realSizeBegin = {g.width * pixelToVP * sizeBegin, g.height * pixelToVP * sizeBegin};
				glm::vec2 realSizeEnd = { g.width * pixelToVP * sizeEnd, g.height * pixelToVP * sizeEnd };

				b.AddParticle(curPos, vel + GM_GetRandomFloat(-1.0f, 1.0f) * velVariation, realSizeBegin, realSizeEnd, colBegin, colEnd, idx + m->metrics->atlasIdx, 0.0f, 0.0f, lifeTime);
			}
		}


	}
}


void GM_ClearScene()
{
	GameState* game = GetGameState();
	GameManager* m = (GameManager*)game->manager;
	m->waitTimer = 0.0f;
	m->attackCycleState = ATTACK_CYCLE_STATES::WAIT_FOR_INPUT;

	m->stats.accumulatedDamage = 0.0f;
	m->stats.critMultiplier = 1.0f;

	for (Ball* b : m->ballList)
	{
		delete b;
	}
	for (Projectile* p : m->projList)
	{
		delete p;
	}

	for (Peg* p : m->pegList)
	{
		delete p;
	}
	for (Character* c : m->enemyList)
	{
		delete c;
	}
	delete m->player;
	delete m->particleHandler;
	delete m->background;

	m->enemyActionList.clear();
	m->ballList.clear();
	m->projList.clear();
	m->pegList.clear();
	m->enemyList.clear();
	m->player = nullptr;
	m->particleHandler = nullptr;
	m->background = nullptr;
}
void GM_FillScene()
{
	GameState* game = GetGameState();
	GameManager* m = (GameManager*)game->manager;
	
	m->background = new Base(glm::vec2(0.0f, 1.0f), glm::vec2(-2.0f, -1.5f), glm::vec2(2.0f, 1.2f));
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
		"###   ###   ######   ###   ######   ###   ######   ###   ###\
		\n   ###   ###   ###   ###   ###   ###   ###   ###   ###   ###\
		\n###   ###   ############   ######   ###   ######   ###   ###\
		\n   ###   ###   #########   ###   ###   ###   ###   ###   ###\
		\n###   ###   #####################   ###   ######   ###   ###\
		\n   ###   ###   ###############   ###   ###   ###   ###   ###\
		\n###   ###   #####################   ###   ######   ###   ###\
		\n   ###   ###   ###############   ###   ###   ###   ###   ###\
		\n###   ###   #####################   ###   ######   ###   ###\
		\n   ###   ###   ###############   ###   ###   ###   ###   ###\
		\n###   ###   #####################   ###   ######   ###   ###\
		\n   ###   ###   ###############   ###   ###   ###   ###   ###\
		\n###   ###   #####################   ###   ######   ###   ###\
		\n   ###   ###   ###   ###   ###   ###   ###   ###   ###   ###\
		\n###   ###   ######   ###   ######   ###   ######   ###   ###\
		\n   ###   ###   ###   ###   ###   ###   ###   ###   ###   ###\
		\n###   ###   ######   ###   ######   ###   ######   ###   ###\
		\n   ###   ###   ###   ###   ###   ###   ###   ###   ###   ###\
		\n###   ###   ######   ###   ######   ###   ######   ###   ###\
		\n   ###   ###   ###   ###   ###   ###   ###   ###   ###   ###\n");
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

		const uint32_t steps = (uint32_t)(lineEnd - first);

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
	if (rd < 0.93f) return ENTITY_TYPE::STANDARD_PEG;
	else if (rd < 0.97f) return ENTITY_TYPE::REFRESH_PEG;
	else return ENTITY_TYPE::CRIT_PEG;
}

void GM_PlaySound(SOUNDS sound, float volume)
{
	if (sound >= SOUNDS::SOUND_NONE) return;
	GameState* s = GetGameState();
	GameManager* m = GM_GetGameManager();
	AU_PlayAudio(s->audio, m->audioFiles.at(sound), volume);
}