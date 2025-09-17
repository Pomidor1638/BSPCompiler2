#include "input.h"

namespace InputManager {

	int numkeys;
	const Uint8* realkeyboardstate;
	Uint8 lastkeyboardstate[SDL_NUM_SCANCODES];

	Uint32 realMouseState, lastMouseState;

	int  realX, realY;
	int  lastX, lastY;
	int deltaX, deltaY;

	bool relativeMode;

	Sint32 mouseWheelX = 0;
	Sint32 mouseWheelY = 0;
	float mouseWheelXPrecise = 0.0f;
	float mouseWheelYPrecise = 0.0f;
	bool wheelMoved = false;

	void GetPosition(int& x, int& y) {
		x = realX;
		y = realY;
	}

	void SetPosition(int x, int y) {
		if (relativeMode)
			return;
		SDL_WarpMouseInWindow(Engine::window, x, y);
		realX = x;
		realY = y;
		lastX = x;
		lastY = y;
		deltaX = 0;
		deltaY = 0;
	}


	bool IM_Init() {

		if (!Engine::window)
			return false;

		realkeyboardstate = SDL_GetKeyboardState(&numkeys);
		if (!realkeyboardstate)
			return false;
		memcpy(lastkeyboardstate, realkeyboardstate, numkeys * sizeof(*realkeyboardstate));

		realMouseState = SDL_GetMouseState(&realX, &realY);
		lastMouseState = realMouseState;

		lastX = realX;
		lastY = realY;
		deltaX = 0;
		deltaY = 0;

		relativeMode = false;
		SDL_SetRelativeMouseMode(SDL_FALSE);

		return true;
	}

	void UpdateMouse() {

		lastMouseState = realMouseState;

		if (relativeMode) {
			realMouseState = SDL_GetRelativeMouseState(&deltaX, &deltaY);
		}
		else {
			realMouseState = SDL_GetMouseState(&realX, &realY);
			deltaX = realX - lastX;
			deltaY = realY - lastY;

			lastX = realX;
			lastY = realY;
		}

	}

	void UpdateKeyboard() { // Calls BEFORE HandleEvents (SDL_PollEvent)
		memcpy(lastkeyboardstate, realkeyboardstate, numkeys * sizeof(*realkeyboardstate));
	}

	void IM_Update() {
		UpdateKeyboard();
		UpdateMouse();
	}

	bool KeyPressed(SDL_Scancode k) {
		return realkeyboardstate[k];
	}
	bool KeyJustPressed(SDL_Scancode k) {
		return !lastkeyboardstate[k] && realkeyboardstate[k];
	}
	bool KeyReleased(SDL_Scancode k) { // Who the fuck use this when it equals !keyPressed(k)?
		return !realkeyboardstate[k];
	}
	bool KeyJustReleased(SDL_Scancode k) {
		return lastkeyboardstate[k] && !realkeyboardstate[k];
	}

	bool MousePressed(int b) {
		return realMouseState & SDL_BUTTON(b);
	}
	bool MouseJustPressed(int b) {
		return !(lastMouseState & SDL_BUTTON(b)) && (realMouseState & SDL_BUTTON(b));
	}
	bool MouseReleased(int b) {
		return !(realMouseState & SDL_BUTTON(b));
	}
	bool MouseJustReleased(int b) {
		return (lastMouseState & SDL_BUTTON(b)) && !(realMouseState & SDL_BUTTON(b));
	}

	Sint32 GetMouseWheelX() {
		return mouseWheelX;
	}

	Sint32 GetMouseWheelY() {
		return mouseWheelY;
	}

	float GetMouseWheelXf() {
		return mouseWheelXPrecise;
	}

	float GetMouseWheelYf() {
		return mouseWheelYPrecise;
	}

	bool HandleKeyboardEvent(const SDL_KeyboardEvent& event)
	{
		switch (event.keysym.scancode) {
		case SDL_SCANCODE_ESCAPE:
			return false;
		}
		return true;
	}

	void HandleMouseWheelEvent(const SDL_MouseWheelEvent& event) {
		Sint32 scroll_x = event.x;
		Sint32 scroll_y = event.y;

		if (event.direction == SDL_MOUSEWHEEL_FLIPPED) {
			scroll_x *= -1;
			scroll_y *= -1;
		}

		mouseWheelX = scroll_x;
		mouseWheelY = scroll_y;

		// Обрабатываем точные значения прокрутки (если доступны)
		mouseWheelXPrecise = event.preciseX;
		mouseWheelYPrecise = event.preciseY;

		if (event.direction == SDL_MOUSEWHEEL_FLIPPED) {
			mouseWheelXPrecise = -mouseWheelXPrecise;
			mouseWheelYPrecise = -mouseWheelYPrecise;
		}

		wheelMoved = true;
	}


	bool IsMouseWheelMoved()
	{
		return wheelMoved;
	}

	bool GetRelativeMode() 
	{
		return SDL_GetRelativeMouseMode();
	}
	bool SetRelativeMode(bool mode) 
	{
		relativeMode = !SDL_SetRelativeMouseMode(mode ? SDL_TRUE : SDL_FALSE);
		return relativeMode;
	}

	void GetRelativePosition(int& x, int& y)
	{
		if (relativeMode) 
		{
			SDL_GetRelativeMouseState(&x, &y);
		}
		else
		{
			x = deltaX;
			y = deltaY;
		}
	}

	void IM_Quit() 
	{

	}

};
