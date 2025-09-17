#pragma once
#include "engine.h"

namespace Renderer
{

	struct Camera {

		vec3_t origin;
		vec3_t angles; // x, y, z - roll, pitch, yaw

		float fov;
		float width;
		float height;
		float far_pl;

		Uint32 flags;

	};

	extern Camera mainCamera;

	void FrustumFromCamera(Camera c);

	void MoveCamera(Camera c);

	bool R_Init();
	void Render();
	void R_Quit();
};