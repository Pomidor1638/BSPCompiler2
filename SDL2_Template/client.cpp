#include "client.h"
#include "render.h"
#include "input.h"
#include "mathlib.h"



namespace Client {

	bool CL_Init() 
	{
		return InputManager::SetRelativeMode(true);
	}

	vec_t positive_mod(vec_t t, vec_t mod, bool block)
	{
		if (block)
		{
			if (t < 0)
			{
				t = 0;
			}
			else
			{
				t = t > mod ? mod : t;
			}
		}
		else
		{
			t = t - int(t / mod) * mod;
			if (t < 0)
				t += mod;
		}
		return t;
	}

	void UpdateAngle(vec3_t& angles)
	{

		int dx, dy;
		InputManager::GetRelativePosition(dx, dy);

		angles[YAW]   -= dx / 180.0 * M_PI / 8;
		angles[PITCH] -= dy / 180.0 * M_PI / 8;

		angles[YAW] = positive_mod(angles[YAW], 2 * M_PI, false);
		angles[PITCH] = positive_mod(angles[PITCH], M_PI, true);
	}

	void CL_Update() 
	{
		vec3_t& angle = Renderer::MainCamera.angles;
		vec3_t& origin = Renderer::MainCamera.origin;

		UpdateAngle(angle);


		vec3_t wishdir{};
		
		double cos_z = cos(angle[2]);
		double sin_z = sin(angle[2]);

		if (InputManager::KeyPressed(SDL_SCANCODE_W))
		{
			wishdir[0] -= sin_z;
			wishdir[1] += cos_z;
		}
		if (InputManager::KeyPressed(SDL_SCANCODE_D))
		{
			wishdir[0] += cos_z;
			wishdir[1] += sin_z;
		}
		if (InputManager::KeyPressed(SDL_SCANCODE_S))
		{
			wishdir[0] += sin_z;
			wishdir[1] -= cos_z;
		}
		if (InputManager::KeyPressed(SDL_SCANCODE_A))
		{
			wishdir[0] -= cos_z;
			wishdir[1] -= sin_z;
		}
		if (InputManager::KeyPressed(SDL_SCANCODE_SPACE))
		{
			wishdir[2] += 1;
		}
		if (InputManager::KeyPressed(SDL_SCANCODE_LSHIFT))
		{
			wishdir[2] -= 1;
		}

		const vec_t speed = 1000;

		origin += wishdir * speed * Engine::deltaTime;


	}

	void CL_Quit() 
	{

	}

}