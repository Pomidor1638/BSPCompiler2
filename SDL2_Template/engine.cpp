#include "engine.h"
#include "input.h"
#include "render.h"
#include "client.h"

#include <functional>

namespace Engine {

	bool api_inited;

	SDL_Window* window;
	SDL_GLContext context;

	bool running;

	std::string bspfilename;

	Map map;

	Uint64 realTime;
	Uint64 lastTime;

	std::string title;
	int width;
	int height;

	Uint64 totalTime;
	double deltaTime;

	bool E_Init() 
	{

		if (!(api_inited = (!SDL_Init(SDL_INIT_EVERYTHING))))  
		{
			return false;
		}

		title = "MegaTitle";

		width = 1000;
		height = 800;

		running = false;

		window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL /*| SDL_WINDOW_FULLSCREEN*/);
		if (!window) {
			return false;
		}

		context = SDL_GL_CreateContext(window);

		if (!context) {
			return false;
		}

		SDL_GL_MakeCurrent(window, context);

		bspfilename = "nigger.bsp";

		if (!map.load(bspfilename))
		{
			printf("Can't open %s\n", bspfilename.c_str());
			return false;
		}
		else
		{
			printf("%s\n", map.toString().c_str());
		}


		realTime = SDL_GetTicks64();
		lastTime = realTime;

		deltaTime = 0;
		totalTime = 0;

		SDL_GL_SetSwapInterval(0);

		running = true;
		return true;
	}

	bool Init(int argc, char** argv) 
	{
		E_Init();

		InputManager::IM_Init();
		Renderer::R_Init();
		Client::CL_Init();

		return false;
	}

	void Quit() 
	{
		if (context)
			SDL_GL_DeleteContext(context);
		if (window)
			SDL_DestroyWindow(window);
		SDL_Quit();
	}

	void ComputeDeltaTime() 
	{

		realTime = SDL_GetTicks64();

		int dt = (realTime - lastTime);

		totalTime += dt;
		deltaTime = dt / 1000.0;
		lastTime = realTime;

	}

	void HandleEvent(const SDL_Event& e)
	{
		switch (e.type)
		{
		case SDL_QUIT:
			running = false;
			return;
		case SDL_MOUSEWHEEL:
			InputManager::HandleMouseWheelEvent(e.wheel);
			break;
		case SDL_KEYDOWN:
			running = InputManager::HandleKeyboardEvent(e.key);
			break;
		default:
			break; 
		}
	}

	void CheckEvents() 
	{
		SDL_Event event{};
		while (SDL_PollEvent(&event))
		{
			HandleEvent(event);
		}
	}

	void Update() 
	{
		InputManager::IM_Update();
		CheckEvents();
		Client::CL_Update();
		Renderer::R_Update();
	}

	void Frame() 
	{

		ComputeDeltaTime();
		Update();
		Renderer::Render();

	}

};