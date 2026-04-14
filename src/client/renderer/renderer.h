// ============================================================================
// render.h
// ============================================================================
#ifndef MEGAGAME_RENDER_H
#define MEGAGAME_RENDER_H

#include <SDL2/SDL.h>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "../../common/common.h"
#include "../../common/bspfile/bspfile.h"
#include "../world/world.h"
#include "../state/state.h"

class Renderer
{
public:

	Renderer() = default;
	~Renderer();

	Renderer& operator=(const Renderer&) = delete;
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&) = delete;

	Renderer(const GameState* s, SDL_Window* w);

	bool init();
	void quit();

	void startProcessEvent();
	void processEvent(const SDL_Event& event);
	void endProcessEvent();



	void render();

	struct Camera
	{
	public:
		glm::vec3 origin{ 0.0f };
		glm::vec3 angles{ 0.0f };

		float fov        = glm::radians(60.0f);
		float aspect     =        4.0f / 3.0f;
		float near_plane =               1.0f;
		float far_plane  =            2000.0f;

		// cache
		glm::mat4 view               { 1.0f };
		glm::mat4 projection         { 1.0f };
		glm::mat4 view_projection    { 1.0f };
		glm::mat4 inv_view_projection{ 1.0f };

		glm::vec3 frustum_points[8]{};


		void updateMatrices();
		void updateFrustumPoints();

		bool in_water = false;

	};

	Camera getCamera() const;
	void   setCamera(const Camera& c);

	bool loadBSPMap(const BSPMap& bspmap);
	void clear();

private:

	bool loadFaces(const BSPMap& bspmap);
	bool loadTextures(const BSPMap& bspmap);

	void updateCameraFrameData();

	void render_start();
	void render_end();


	// 3d render

	void render_face(int face_id);
	void render_leaf(int leaf_id);

	void bsp_traverse(int headnode);

	void render_world();

	void render_main_start();
	void render_main_end();
	void render_main();

	// hud, ui, etc.

	// HUD

	void render_HUD_start();
	void render_HUD_end();
	void render_HUD();

	// UI
	void render_UI_start();
	void render_UI_end();
	void render_UI();

	void render_secondary();

	bool initialized = false;

	Camera camera{};

	struct
	{
		int drawn_faces = 0;
		int drawn_leafs = 0;
		int traversed_nodes = 0;

		void clear()
		{
			drawn_faces = 0;
			drawn_leafs = 0;
			traversed_nodes = 0;
		}

	} debug;

	// Render Geometry
	struct BufferContext
	{
		GLuint vao = 0;
		GLuint vbo = 0;
		GLuint ebo = 0;
	};

	struct R_Face
	{
		int plane_id = -1;
		GLuint texturenum = 0;

		GLuint first_index = 0;
		GLuint num_indexes = 0;
	};

	struct R_Texture
	{
		std::string name   = {};
		int         width  = -1;
		int         height = -1;
		int         gl_id  =  0;
	};

	std::vector<GLuint> texture_ids{};

	BufferContext faces_buffer{};
	std::vector<R_Texture> textures{};

	std::vector<R_Face> faces{};

	const GameState* game_state = nullptr;

	// api thing
	SDL_Window* window = nullptr;
	SDL_GLContext   gl_context = nullptr;

};

#endif // MEGAGAME_RENDER_H