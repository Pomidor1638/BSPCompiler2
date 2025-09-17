
#include "render.h"

#include <SDL_opengl.h>
#pragma comment(lib, "opengl32.lib")

namespace Renderer
{

	Camera mainCamera;

	void FrustumFromCamera(Camera c) {

		const float hw = c.width / 2;
		const float hh = c.height / 2;

		const float near_pl = hw / tan(c.fov / 2);

		glFrustum(-hw, hw, -hh, hh, near_pl, c.far_pl);
	}

	void TranslateCamera(const vec3_t& origin)
	{
		glTranslated(-origin[0], -origin[1], -origin[2]);
	}

	void RotateCamera(const vec3_t& angles)
	{
		glRotated(-radiansToDegrees(angles[PITCH]), 1, 0, 0);
		glRotated(-radiansToDegrees(angles[YAW]), 0, 0, 1);
		glRotated(-radiansToDegrees(angles[ROLL]), 0, 1, 0);
	}

	void MoveCamera(Camera c)
	{
		// Translate and Rotate order explaining:
		// 1 - rotate model coordinates
		// 2 - translate one's
		RotateCamera(c.angles);
		TranslateCamera(c.origin);
	}

	bool R_Init()
	{
		if (!Engine::context)
			return false;

		mainCamera = Camera{};

		mainCamera.fov = M_PI / 2;
		mainCamera.height = 2;
		mainCamera.width = 2 * (float(Engine::width) / Engine::height);

		mainCamera.far_pl = 10000;

		FrustumFromCamera(mainCamera);

		return true;
	}

	void RenderBegin()
	{
		glClear(GL_COLOR_BUFFER_BIT/* | GL_DEPTH_BUFFER_BIT*/);
		glClearColor(0.1, 0.3, 0.2, 1.0);
		glPushMatrix();
	}

	void drawFace(const face_t& face) {
		glBegin(/*GL_POLYGON*/GL_LINE_LOOP);

		glColor3f(1, 1, 1);

		for (int i = 0; i < face.numpoints; i++) {
			//const texinfo_t& texinfo = Engine::map.texinfos[face.texinfonum];

			int index = Engine::map.vindexes[face.firstpoint + i];
			vec3_t& v = Engine::map.vertexes[index];

#ifdef DOUBLE_VEC_T
			glVertex3dv((const GLdouble*)&v[0]);
#else
			glVertex3fv((const GLfloat*)&v[0]);
#endif
		}
		glEnd();
	}

	void drawLeaf(int leafnum) {

		const leaf_t& leaf = Engine::map.leafs[leafnum];

		for (int i = 0; i < leaf.numfaces; i++) {
			const face_t& face = Engine::map.faces[leaf.firstface + i];
			drawFace(face);
		}

	}

	void painters_algorithm(int nodenum) {
		if (nodenum < 0) {
			drawLeaf(-(nodenum + 1));
			return;
		}

		const node_t& node = Engine::map.nodes[nodenum];
		
		const plane_t& plane = Engine::map.planes[node.planenum];

		const bool t = (plane.normal * mainCamera.origin - plane.dist) > 0;

		painters_algorithm(node.children[ t]);
		painters_algorithm(node.children[!t]);
	}

	void RenderWorld(Camera cam)
	{
		//glPushMatrix();

		MoveCamera(cam);
		
		//int headnode = Engine::map.models[0].headnode[0];

		//painters_algorithm(headnode);

		for (const auto& x : Engine::map.faces) {
			drawFace(x);
		}
			

		//glPopMatrix();
	}

	void RenderHUD()
	{

	}

	void RenderEnd()
	{
		glPopMatrix();
		SDL_GL_SwapWindow(Engine::window);
	}

	void Render()
	{
		RenderBegin();
		RenderWorld(mainCamera);
		RenderHUD();
		RenderEnd();
	}

	void R_Quit()
	{

	}
};