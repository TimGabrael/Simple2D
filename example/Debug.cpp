#include "Debug.h"
#include "GameState.h"



std::vector<glm::vec2> Debug_OutlineClicker()
{
	static std::vector<glm::vec2> positions;
	static bool lastMouseButton = GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);

	bool mouseButton = GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
	if (mouseButton && mouseButton != lastMouseButton)
	{
		GameState* state = GetGameState();

		state->mouseX;
	}
	lastMouseButton = mouseButton;
	return positions;
}