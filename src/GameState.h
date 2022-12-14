#pragma once 
#include "Graphics/Renderer.h"
#include "Util/Assets.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Physics/Physics.h"
#include "Audio/AudioManager.h"
#define PRESSED_KEY_BUFFER 255
#define TIME_STEP 1.0f / 60.0f

struct BaseGameManager
{
	virtual void RenderCallback(struct GameState* state, float dt) = 0;
	virtual void Update(float dt) = 0;

	virtual void PreUpdate(float dt) {};
	virtual void PostUpdate(float dt) {};
	virtual void OnWindowPositionChanged(int x, int y) {};
	virtual void OnWindowResize(int w, int h) {};
	virtual void OnKey(int key, int scancode, int action, int mods) {};
	virtual void OnMouseButton(int button, int action, int mods) {};
	virtual void OnMousePositionChanged(float x, float y, float dx, float dy) {};
};


struct GameState
{
	struct Renderer* renderer;
	struct Scene* scene;
	struct PhysicsScene* physics;
	struct AudioManager* audio;
	BaseGameManager* manager;
	struct GLFWwindow* window;
	int* pressedKeys;
	int numPressedKeys;

	uint32_t winX;
	uint32_t winY;
	uint32_t winWidth;
	uint32_t winHeight;
	uint32_t swapChainInterval;
	uint32_t numGamepads;
	float mouseX;
	float mouseY;
	float accumulatedTime;
	float aspectRatio;
	float tickMultiplier;
	bool hasFocus;
	bool isFullscreen;
	bool isMouseCaptured;
};

GameState* CreateGameState(const char* windowName, uint32_t windowWidth, uint32_t windowHeight, float gravity, int concurrentAudio);
void CleanUpGameState(GameState* state);

GameState* GetGameState();
void SetFullscreen(GameState* state, int monitorIdx, int* width, int* height);
void SetWindowed(GameState* state, int width, int height);

// first bit signals just pressed, second bit signals is down
uint8_t GetKey(int key);
uint8_t GetMouseButton(int button);

void UpateGameState();

void UpdateGameStep();