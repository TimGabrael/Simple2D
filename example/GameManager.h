#pragma once
#include "GameState.h"
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


	NUM_SPRITES,
};

struct GameManager : public BaseGameManager
{
	virtual void RenderCallback(GameState* state) override;
	virtual void Update(float dt) override;

	virtual void PostUpdate(float dt) override;

	virtual void OnWindowPositionChanged(int x, int y) override;
	virtual void OnWindowResize(int w, int h) override;
	virtual void OnKey(int key, int scancode, int action, int mods) override;
	virtual void OnMouseButton(int button, int action, int mods) override;
	virtual void OnMousePositionChanged(float x, float y, float dx, float dy) override;

	std::vector<SceneObject*> ballList;
	glm::vec2 vpStart;
	glm::vec2 vpEnd;
	glm::mat4 viewProj;
	AtlasTexture* atlas = nullptr;
	FontMetrics* metrics = nullptr;
};

GameManager* GM_CreateGameManager(GameState* state);

GameManager* GM_GetGameManager();


float GetRandomFloat(float start, float end);