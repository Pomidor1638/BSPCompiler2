#pragma once

#include <SDL.h>
#include <string>
#include "world.h"

//
//struct SDL_Window;
//typedef void* SDL_GLContext;

namespace Engine {

	extern SDL_Window* window;
	extern SDL_GLContext context; 

	extern bool running;

	extern Map map;

	extern std::string title;
	extern int width;
	extern int height;

	extern Uint64 totalTime;
	extern double deltaTime;

	bool Init(int argc, char** argv);
	void Quit();
	void Frame();

};