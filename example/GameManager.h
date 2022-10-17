#pragma once
#include "GameState.h"
#include "Audio/AudioManager.h"
#define PADDING b2_polygonRadius

enum SPRITES
{
	DISH,
	DISH_2,
	DISH_PILE,
	BOWL,
	APPLE_PIE,
	APPLE_PIE_DISH,
	BREAD,
	BREAD_DISH,
	BAGUETTE,
	BAGUETTE_DISH,
	BUN,
	BUN_DISH,
	BACON,
	BACON_DISH,
	BURGER,
	BURGER_DISH,
	BURGER_NAPKIN,
	BURRITO,
	BURRITO_DISH,
	BAGEL,
	BAGEL_DISH,
	CHEESECAKE,
	CHEESECAKE_DISH,
	CHEESEPUFF,
	CHEESEPUFF_BOWL,
	CHOCOLATE,
	CHOCOLATE_DISH,
	COOKIES,
	COOKIES_DISH,
	CHOCOLATECAKE,
	CHOCOLATECAKE_DISH,
	CURRY,
	CURRY_DISH,
	DONUT,
	DONUT_DISH,
	DUMPLINGS,
	DUMPLINGS_DISH,
	FRIEDEGG,
	FRIEDEGG_DISH,
	EGGSALAD,
	EGGSALAD_BOWL,
	EGGTART,
	EGGTART_DISH,
	FRENCHFRIES,
	FRENCHFRIES_DISH,
	FRUITCAKE,
	FRUITCAKE_DISH,
	GARLICBREAD,
	GARLICBREAD_DISH,
	GIANTGUMMYBEAR,
	GIANTGUMMYBEAR_DISH,
	GINGERBREADMAN,
	GINGERBREADMAN_DISH,
	HOTDOG,
	HOTDOG_SAUCE,
	HOTDOG_DISH,
	ICECREAME,
	ICECREAME_BOWL,
	JELLY,
	JELLY_DISH,
	JAM,
	JAM_DISH,
	LEMONPIE,
	LEMONPIE_DISH,
	LOAFBREAD,
	LOAFBREAD_DISH,
	MACNCHEESE,
	MACNCHEESE_DISH,
	MEATBALL,
	MEATBALL_DISH,
	NACHO,
	NACHO_DISH,
	OMLET,
	OMLET_DISH,
	PUDDING,
	PUDDING_DISH,
	POTATOCHIPS,
	POTATOCHIPS_BOWL,
	PANCAKES,
	PANCAKES_DISH,
	PIZZA,
	PIZZA_DISH,
	POPCORN,
	POPCORN_BOWL,
	ROASTEDCHICKEN,
	ROASTEDCHICKEN_DISH,
	RAMEN,
	SALMON,
	SALMON_DISH,
	STRAWBERRYCAKE,
	STRAWBERRYCAKE_DISH,
	SANDWICH,
	SANDWICH_DISH,
	SPAGHETTI,
	STEAK,
	STEAK_DISH,
	SUSHI,
	SUSHI_DISH,
	TACO,
	TACO_DISH,
	WAFFLE,
	WAFFLE_DISH,
	FOX_IDLE_0,
	FOX_IDLE_1,
	FOX_IDLE_2,
	FOX_IDLE_3,
	FOX_IDLE_4,
	FOX_LAY_0,
	FOX_LAY_1,
	FOX_LAY_2,
	FOX_LAY_3,
	FOX_LAY_4,
	FOX_LAY_5,
	FOX_LAY_6,
	FOX_LEAP_0,
	FOX_LEAP_1,
	FOX_LEAP_2,
	FOX_LEAP_3,
	FOX_LEAP_4,
	FOX_LEAP_5,
	FOX_LEAP_6,
	FOX_LEAP_7,
	FOX_LEAP_8,
	FOX_LEAP_9,
	FOX_LEAP_10,
	FOX_LOOK_0,
	FOX_LOOK_1,
	FOX_LOOK_2,
	FOX_LOOK_3,
	FOX_LOOK_4,
	FOX_LOOK_5,
	FOX_LOOK_6,
	FOX_LOOK_7,
	FOX_LOOK_8,
	FOX_LOOK_9,
	FOX_LOOK_10,
	FOX_LOOK_11,
	FOX_LOOK_12,
	FOX_LOOK_13,
	FOX_SHOCK_0,
	FOX_SHOCK_1,
	FOX_SHOCK_2,
	FOX_SHOCK_3,
	FOX_SHOCK_4,
	FOX_SLEEP_0,
	FOX_SLEEP_1,
	FOX_SLEEP_2,
	FOX_SLEEP_3,
	FOX_SLEEP_4,
	FOX_SLEEP_5,
	FOX_WALK_0,
	FOX_WALK_1,
	FOX_WALK_2,
	FOX_WALK_3,
	FOX_WALK_4,
	FOX_WALK_5,
	FOX_WALK_6,
	FOX_WALK_7,


	NUM_SPRITES,
};

enum SOUNDS
{
	SOUND_CLACK,
	NUM_SOUNDS,
};


struct GameManager : public BaseGameManager
{
	void OnAllBallsFell();

	virtual void RenderCallback(GameState* state) override;
	virtual void Update(float dt) override;

	virtual void PostUpdate(float dt) override;

	virtual void OnWindowPositionChanged(int x, int y) override;
	virtual void OnWindowResize(int w, int h) override;
	virtual void OnKey(int key, int scancode, int action, int mods) override;
	virtual void OnMouseButton(int button, int action, int mods) override;
	virtual void OnMousePositionChanged(float x, float y, float dx, float dy) override;

	std::vector<WavFile*> audioFiles;

	std::vector<SceneObject*> ballList;
	std::vector<SceneObject*> pegList;
	std::vector<SceneObject*> projList;
	SceneObject* background = nullptr;
	SceneObject* player = nullptr;
	SceneObject* particleHandler = nullptr;
	glm::vec2 vpStart;
	glm::vec2 vpEnd;
	glm::vec2 targetDir;
	float startVelocity;
	glm::mat4 viewProj;
	AtlasTexture* atlas = nullptr;
	FontMetrics* metrics = nullptr;
	uint32_t accumulatedDamage = 0;
	bool ballsAreFalling = false;
};

GameManager* GM_CreateGameManager(GameState* state);

GameManager* GM_GetGameManager();


float GM_GetRandomFloat(float start, float end);


void GM_AddParticle(const glm::vec2& pos, const glm::vec2& vel, const glm::vec2& sizeBegin, const glm::vec2& sizeEnd, uint32_t colBegin, uint32_t colEnd, SPRITES sprite, float rotationBegin, float rotationEnd, float lifeTime);

void GM_FillScene();

void GM_CreateFieldFromCharacters(struct Base* b, const char* field);


void GM_PlaySound(SOUNDS sound, float volume);