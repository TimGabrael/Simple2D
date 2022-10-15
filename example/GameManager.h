#pragma once
#include "GameState.h"

struct GameManager : public BaseGameManager
{
	virtual void RenderCallback(GameState* state) override;
	virtual void Update(float dt) override;


	virtual void OnWindowPositionChanged(int x, int y) override;
	virtual void OnWindowResize(int w, int h) override;
	virtual void OnKey(int key, int scancode, int action, int mods) override;
	virtual void OnMouseButton(int button, int action, int mods) override;
	virtual void OnMousePositionChanged(float x, float y, float dx, float dy) override;

	AtlasTexture* atlas = nullptr;
	FontMetrics* metrics = nullptr;
};

GameManager* GM_CreateGameManager(GameState* state);