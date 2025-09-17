#pragma once
#include "engine.h"

namespace InputManager {

	void GetPosition(int& x, int& y);
	void GetRelativePosition(int& x, int& y); // only relative mode
	void SetPosition(int  x, int  y);

	bool IM_Init();
	void IM_Quit();
	void IM_Update();

	bool KeyPressed(SDL_Scancode k);
	bool KeyJustPressed(SDL_Scancode k);
	bool KeyReleased(SDL_Scancode k);
	bool KeyJustReleased(SDL_Scancode k);

	bool MousePressed(int b);
	bool MouseJustPressed(int b);
	bool MouseReleased(int b);
	bool MouseJustReleased(int b);

	Sint32 GetMouseWheelX();
	Sint32 GetMouseWheelY(); 
	float GetMouseWheelXf(); 
	float GetMouseWheelYf(); 

	bool HandleKeyboardEvent(const SDL_KeyboardEvent& event);
	void HandleMouseWheelEvent(const SDL_MouseWheelEvent& event);
	bool IsMouseWheelMoved();

	bool GetRelativeMode();
	bool SetRelativeMode(bool mode);

};
