
#include "render.h"

#include <SDL_opengl.h>
#pragma comment(lib, "opengl32.lib")

namespace Renderer
{

	Camera MainCamera;

	int CurNode[MAX_MAP_HULLS];
	GLfloat CurColor[3];


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



	void RenderBegin()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glPushMatrix();
	}

	void drawFace(const face_t& face, bool wire = false) {

		const texinfo_t& texinfo = Engine::map.texinfos[face.texinfonum];
		const texture_t& texture = Engine::map.textures[texinfo.texturenum];

		if (!wire)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, texture.gl_texturenum);

			glBegin(GL_POLYGON);
			glColor3fv(CurColor);
		}
		else
		{
			glBegin(GL_LINE_LOOP);
			glColor3f(1, 0, 0);
		}
		for (int i = 0; i < face.numpoints; i++) {
			int index = Engine::map.vindexes[face.firstpoint + i];
			vec3_t& vertex = Engine::map.vertexes[index];
			if (!wire)
			{
				// Вычисляем текстурные координаты
				float u = (vertex * texinfo.u) + texinfo.uoffset;
				float v = (vertex * texinfo.v) + texinfo.voffset;

				glTexCoord2f(u, v);
			}

#ifdef DOUBLE_VEC_T
			glVertex3dv((const GLdouble*)&v[0]);
#else
			glVertex3fv((const GLfloat*)&vertex[0]);
#endif
		}

		glEnd();
		if (!wire)
			glDisable(GL_TEXTURE_2D);
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
		const bool is_front = plane.dot(MainCamera.origin) > 0;

		painters_algorithm(node.children[is_front]);
		painters_algorithm(node.children[!is_front]);
	}

	void RenderWorld(Camera cam)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		FrustumFromCamera(cam);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		MoveCamera(cam);

		int headnode = Engine::map.models[0].headnode[0];
		painters_algorithm(headnode);
	}


	void RenderHUD()
	{

	}

	void RenderEnd()
	{
		glPopMatrix();
		SDL_GL_SwapWindow(Engine::window);
	}

	void UpdateDrawNode() 
	{

		int nodenum = Engine::map.models[0].headnode[0];

		const vec3_t& origin = MainCamera.origin;

		while (nodenum >= 0) {
			const node_t& node = Engine::map.nodes[nodenum];
			const plane_t& plane = Engine::map.planes[node.planenum];
			const bool isFront = plane.dot(origin) > 0;
			nodenum = node.children[!isFront];
		}

		CurNode[0] = -(nodenum + 1);
	}

	void R_Update() 
	{
		UpdateDrawNode();

		const leaf_t& leaf = Engine::map.leafs[CurNode[0]];

		switch (leaf.contents) {
		case CONTENTS_WATER:
			CurColor[0] = 0.1;
			CurColor[1] = 0.3;
			CurColor[2] = 0.5;
			break;
		default:
			CurColor[0] = 1.0;
			CurColor[1] = 1.0;
			CurColor[2] = 1.0;
			break;
		}

	}

	void Render()
	{
		RenderBegin();
		RenderWorld(MainCamera);
		RenderHUD();
		RenderEnd();
	}

	void setupFog() {
		glEnable(GL_FOG);

		GLfloat fogColor[4] = { 0.2f, 0.0f, 0.1f, 1.0f };
		glFogfv(GL_FOG_COLOR, fogColor);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_START, MainCamera.far_pl * 0.3f);
		glFogf(GL_FOG_END, MainCamera.far_pl);
		glHint(GL_FOG_HINT, GL_FASTEST);

		glClearColor(fogColor[0], fogColor[1], fogColor[2], fogColor[3]);
	}



	bool R_Init()
	{
		if (!Engine::context)
			return false;

		MainCamera = Camera{};
		MainCamera.fov = M_PI / 2;
		MainCamera.height = 2;
		MainCamera.width = 2 * (float(Engine::width) / Engine::height);
		MainCamera.far_pl = 5000;

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		FrustumFromCamera(MainCamera);

		setupFog();

		return true;
	}



	void R_Quit()
	{

	}
};