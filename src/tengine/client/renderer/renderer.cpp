#include "renderer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include "protocol/protocol.h"
#include "renderer/imgui/imgui.h"
#include "state/state.h"
#include "utils/logger/logger.h"
#include <SDL_main.h>
#include <ostream>
namespace tungsten::renderer
{
	namespace
	{
		void getDrawableSize(SDL_Window* window, int& width, int& height)
		{
			width = 1;
			height = 1;

			if (!window)
				return;

			SDL_GL_GetDrawableSize(window, &width, &height);

			if (width <= 0)
				width = 1;
			if (height <= 0)
				height = 1;
		}
	}

	Renderer::Renderer(client::ClientState* s, SDL_Window* w)
		: initialized{ false }
		, window{ w }
		, game_state{ s }
	{}

	Renderer::~Renderer()
	{
		quit();
	}

	void Renderer::quit()
	{
		if (!initialized)
			return;

		clear();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		if (gl_context)
		{
			SDL_GL_DeleteContext(gl_context);
			gl_context = nullptr;
		}

		initialized = false;
	}

	bool InitImGui(SDL_Window* window, SDL_GLContext gl_context)
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();

		if (!ImGui_ImplSDL2_InitForOpenGL(window, gl_context))
		{
			ImGui::DestroyContext();
			return false;
		}

		if (!ImGui_ImplOpenGL3_Init("#version 130"))
		{
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();
			return false;
		}

		return true;
	}

	bool Renderer::init()
	{
		if (!window || !game_state)
		{
			return false;
		}

		gl_context = SDL_GL_CreateContext(window);
		if (!gl_context)
		{
			tungsten::logger::gLog << SDL_GetError() << std::endl;
			return false;
		}

		SDL_GL_MakeCurrent(window, gl_context);

		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
		{
			SDL_GL_DeleteContext(gl_context);
			gl_context = nullptr;
			return false;
		}

		if (!InitImGui(window, gl_context))
		{
			SDL_GL_DeleteContext(gl_context);
			gl_context = nullptr;
			return false;
		}

		SDL_GL_SetSwapInterval(0);

		initialized = true;
		return true;
	}


	bool Renderer::loadFaces(const BSPMap& bspmap)
	{
		struct RenderVertex
		{
			glm::vec3 position;
			glm::vec2 uv;
		};

		std::vector<RenderVertex> vertices;
		std::vector<GLuint>       indices;

		size_t n = bspmap.faces.size();
		if (n > MAX_MAP_FACES)
		{
			return false;
		}

		faces.resize(n);

		for (size_t i = 0; i < n; i++)
		{
			const auto& face = bspmap.faces[i];

			if (face.texinfonum >= bspmap.texture_infos.size())
			{
				continue;
			}

			const auto& texinfo = bspmap.texture_infos[face.texinfonum];
			if (texinfo.texnum >= bspmap.textures.size())
			{
				continue;
			}

			const auto& vecs = texinfo.vecs;
			const BSPTexture& texture = bspmap.textures[texinfo.texnum];

			R_Face& r_face = faces[i];
			r_face.plane_id = face.planenum;

			r_face.first_index = static_cast<GLuint>(indices.size());
			r_face.texturenum = texinfo.texnum;

			glm::vec3 u(vecs[0][0], vecs[0][1], vecs[0][2]);
			float u_offset = vecs[0][3];

			glm::vec3 v(vecs[1][0], vecs[1][1], vecs[1][2]);
			float v_offset = vecs[1][3];

			GLuint firstVertexIndex = static_cast<GLuint>(vertices.size());

			for (size_t j = 0; j < face.numpoints; j++)
			{
				size_t vertex_idx = bspmap.vertex_indexes[face.firstpoint + j];
				const auto& pos = bspmap.vertexes[vertex_idx];

				RenderVertex vertex;
				vertex.position = glm::vec3(pos.v[0], pos.v[1], pos.v[2]);

				vertex.uv.x = glm::dot(u, vertex.position) / texture.width + u_offset;
				vertex.uv.y = glm::dot(v, vertex.position) / texture.height + v_offset;



				vertices.push_back(vertex);
			}

			for (size_t j = 1; j < face.numpoints - 1; j++)
			{
				indices.push_back(firstVertexIndex);
				indices.push_back(firstVertexIndex + j);
				indices.push_back(firstVertexIndex + j + 1);
			}

			r_face.num_indexes = static_cast<GLuint>(indices.size() - r_face.first_index);
		}

		if (vertices.empty() || indices.empty())
		{
			return false;
		}

		glGenVertexArrays(1, &faces_buffer.vao);
		glGenBuffers(1, &faces_buffer.vbo);
		glGenBuffers(1, &faces_buffer.ebo);

		glBindVertexArray(faces_buffer.vao);

		glBindBuffer(GL_ARRAY_BUFFER, faces_buffer.vbo);
		glBufferData
		(
			GL_ARRAY_BUFFER,
			vertices.size() * sizeof(RenderVertex),
			vertices.data(),
			GL_STATIC_DRAW
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_buffer.ebo);
		glBufferData
		(
			GL_ELEMENT_ARRAY_BUFFER,
			indices.size() * sizeof(GLuint),
			indices.data(),
			GL_STATIC_DRAW
		);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, sizeof(RenderVertex), (void*)offsetof(RenderVertex, position));

		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, sizeof(RenderVertex), (void*)offsetof(RenderVertex, uv));

		glBindVertexArray(0);

		return true;
	}

	bool Renderer::loadTextures(const BSPMap& bspmap)
	{
		const size_t n = bspmap.textures.size();
		if (n == 0)
			return true;

		textures.resize(n);
		texture_ids.resize(n);

		glGenTextures(static_cast<GLsizei>(n), texture_ids.data());
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		for (size_t i = 0; i < n; i++)
		{
			auto& texture = textures[i];
			const auto& bsp_texture = bspmap.textures[i];

			texture.gl_id = texture_ids[i];
			texture.name = bsp_texture.name;
			texture.width = bsp_texture.width;
			texture.height = bsp_texture.height;

			if (bsp_texture.data.empty() || texture.width <= 0 || texture.height <= 0)
				continue;

			GLenum format = 0;
			GLenum internalFormat = 0;

			switch (bsp_texture.color_type)
			{
			case 1: // Gray
				format = GL_RED;
				internalFormat = GL_R8;
				break;

			case 2: // Gray + Alpha
				format = GL_RG;
				internalFormat = GL_RG8;
				break;

			case 3: // RGB
				format = GL_RGB;
				internalFormat = GL_RGB8;
				break;

			case 4: // RGBA
				format = GL_RGBA;
				internalFormat = GL_RGBA8;
				break;

			default:
				continue;
			}

			glBindTexture(GL_TEXTURE_2D, texture.gl_id);

			glTexImage2D
			(
				GL_TEXTURE_2D,
				0,
				internalFormat,
				texture.width,
				texture.height,
				0,
				format,
				GL_UNSIGNED_BYTE,
				bsp_texture.data.data()
			);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glGenerateMipmap(GL_TEXTURE_2D);

			GLenum err = glGetError();
			if (err != GL_NO_ERROR)
			{
				// сюда лог
			}
		}

		glBindTexture(GL_TEXTURE_2D, 0);
		return true;
	}

	void Renderer::clear()
	{
		if (faces_buffer.vao)
		{
			glDeleteVertexArrays(1, &faces_buffer.vao);
			faces_buffer.vao = 0;
		}
		if (faces_buffer.vbo)
		{
			glDeleteBuffers(1, &faces_buffer.vbo);
			faces_buffer.vbo = 0;
		}
		if (faces_buffer.ebo)
		{
			glDeleteBuffers(1, &faces_buffer.ebo);
			faces_buffer.ebo = 0;
		}

		// Текстуры
		if (!texture_ids.empty())
		{
			glDeleteTextures(static_cast<GLsizei>(texture_ids.size()), texture_ids.data());
			texture_ids.clear();
		}

		textures.clear();
		faces.clear();

	}

	bool Renderer::loadBSPMap(const BSPMap& bspmap)
	{
		clear();
		std::function<bool()> load_funcs[]
		{
			[&]() { return loadTextures(bspmap); },
			[&]() { return loadFaces(bspmap); },
		};

		for (auto& func : load_funcs)
		{
			if (!func())
			{
				return false;
			}
		}

		return true;
	}


	Renderer::Camera Renderer::getCamera() const
	{
		return camera;
	}

	void Renderer::setCamera(const Camera& c)
	{
		camera = c;
	}

	void Renderer::startProcessEvent()
	{}
	void Renderer::processEvent(const SDL_Event& event)
	{
		if (!initialized)
			return;

		ImGui_ImplSDL2_ProcessEvent(&event);
	}
	void Renderer::endProcessEvent()
	{}


	void Renderer::Camera::updateMatrices()
	{
		projection = glm::perspective
		(
			fov,
			aspect,
			near_plane,
			far_plane
		);

		view = glm::mat4{ 1.0f };
		view = glm::rotate(view, -angles.x, { 1, 0, 0 });
		view = glm::rotate(view, -angles.z, { 0, 0, 1 });
		view = glm::rotate(view, -angles.y, { 0, 1, 0 });
		view = glm::translate(view, -origin);

		view_projection = projection * view;
		inv_view_projection = glm::inverse(view_projection);
	}

	void Renderer::Camera::updateFrustumPoints()
	{
		static const glm::vec4 ndc[8] =
		{
			{-1, -1, -1, 1}, // near
			{ 1, -1, -1, 1},
			{ 1,  1, -1, 1},
			{-1,  1, -1, 1},

			{-1, -1,  1, 1}, // far
			{ 1, -1,  1, 1},
			{ 1,  1,  1, 1},
			{-1,  1,  1, 1}
		};

		for (int i = 0; i < 8; i++)
		{
			glm::vec4 world = inv_view_projection * ndc[i];
			world /= world.w;
			frustum_points[i] = glm::vec3(world);
		}
	}


	void Renderer::updateCameraFrameData()
	{
		int width = 1, height = 1;
		getDrawableSize(window, width, height);

		camera.aspect = static_cast<float>(width) / static_cast<float>(height);
		camera.updateMatrices();
		camera.updateFrustumPoints();

		camera.in_water = false;

		const auto& world = game_state->world;
		const auto* cur_leaf = world.cur_leaf[0].leaf;

		if (!cur_leaf)
			return;

		switch (cur_leaf->contents)
		{
		case CONTENTS_WATER:
		case CONTENTS_LAVA:
		case CONTENTS_SLIME:
		case CONTENTS_SKY:
			camera.in_water = true;
			break;

		default:
			camera.in_water = false;
			break;
		}
	}



	void Renderer::render_start()
	{
		int width = 1, height = 1;
		getDrawableSize(window, width, height);
		glViewport(0, 0, width, height);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		glEnable(GL_BLEND);
		glEnable(GL_TEXTURE_2D);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		debug.clear();
	}

	void Renderer::render_end()
	{
		SDL_GL_SwapWindow(window);
	}


	void Renderer::render_face(int face_id)
	{
		const auto& face = faces[face_id];

		glBindTexture(GL_TEXTURE_2D, texture_ids[face.texturenum]);
		glDrawElements
		(
			GL_TRIANGLES,
			face.num_indexes,
			GL_UNSIGNED_INT,
			(void*)(face.first_index * sizeof(GLuint))
		);

		debug.drawn_faces++; // takes into account iteration
	}

	void Renderer::render_leaf(int leaf_id)
	{
		if (!leaf_id) // don't draw solid
			return;

		const auto& world = game_state->world;
		const auto& s = world.bsp_geometry;
		const auto& leaf = s.leafs[leaf_id];
		const auto& f = leaf.face_range;

		bool is_water;
		switch (leaf.contents)
		{
		case CONTENTS_WATER:
		case CONTENTS_LAVA:
		case CONTENTS_SLIME:
		case CONTENTS_SKY:
			is_water = true;
			break;
		default:
			is_water = false;
			break;
		}

		if (is_water && !camera.in_water)
			return;

		for (size_t i = 0; i < f.num; i++)
		{
			size_t f_idx = i + f.first;
			render_face(f_idx);
		}
		debug.drawn_leafs++;
	}

	void Renderer::bsp_traverse(int node_id)
	{
		const auto& world = game_state->world;
		const auto& s = world.bsp_geometry;

		if (node_id < 0)
		{
			render_leaf(~node_id);
			return;
		}

		int counts[3] = { 0, 0, 0 };

		const auto& frustum_points = camera.frustum_points;
		const auto& node = s.nodes[node_id];
		const auto& plane = s.planes[node.plane_id];

		for (int i = 0; i < 8; i++)
		{
			float t = plane.distTo(frustum_points[i]);

			if (t > 0) counts[0]++;
			else if (t < 0) counts[1]++;
			else            counts[2]++;

		}

		bool t = plane.distTo(camera.origin) >= 0;
		debug.traversed_nodes++;
		if (counts[t])
		{
			bsp_traverse(node.children[t]);
		}
		if (counts[!t])
		{
			bsp_traverse(node.children[!t]);
		}
	}


	void Renderer::render_world()
	{
		const auto& s = game_state->world.bsp_geometry;
		const int headnode = s.headnodes[0];

		if (s.leafs.empty())
			return;

		if (headnode >= 0 && static_cast<size_t>(headnode) >= s.nodes.size())
			return;

		glBindVertexArray(faces_buffer.vao);
		bsp_traverse(headnode);
		glBindVertexArray(0);
	}


	void Renderer::render_main_start()
	{
		updateCameraFrameData();

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glm::value_ptr(camera.projection));

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(camera.view));

	}
	void Renderer::render_main_end()
	{}

	void Renderer::render_entities()
	{}

	void Renderer::render_debug_entities()
	{
		const auto& snapshot = game_state->latest_snapshot;

		glDisable(GL_TEXTURE_2D);
		glColor3f(1.0f, 0.0f, 0.0f);

		for (int i = 1; i < snapshot.entity_count; ++i)
		{
			const auto& e = snapshot.entities[i];

			glPushMatrix();
			glTranslatef(e.origin[0], e.origin[1], e.origin[2]);

			glBegin(GL_LINES);
				glVertex3f(-16, 0, 0); glVertex3f(16, 0, 0);
				glVertex3f(0, -16, 0); glVertex3f(0, 16, 0);
				glVertex3f(0, 0, -16); glVertex3f(0, 0, 16);
			glEnd();

			glPopMatrix();
		}

		glColor3f(1.0f, 1.0f, 1.0f);
		glEnable(GL_TEXTURE_2D);
	}


	void Renderer::render_main()
	{
		render_main_start();

		render_world();
	
		render_debug_entities();
		
		render_main_end();
	}


	void Renderer::render_HUD_start()
	{}

	void Renderer::render_HUD_end()
	{}

	void Renderer::render_HUD()
	{
		render_HUD_start();



		render_HUD_end();
	}

	void Renderer::render_UI_start()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		//game_state->ui_cmd = client::UICommand::none;
	}
	void Renderer::render_UI_end()
	{

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}


	void Renderer::render_UI_debug()
	{
		
		const float dt = ImGui::GetIO().DeltaTime;
		const float fps = (dt > 0.0f) ? (1.0f / dt) : 0.0f;
		
		if (
			ImGui::Begin
			(
				"Debug render", nullptr,
				ImGuiWindowFlags_NoInputs |
				ImGuiWindowFlags_NoDecoration |
				ImGuiWindowFlags_NoNav
			)
		) {
			ImGui::Text("Drawn leafs: %6d / %6d",
				debug.drawn_leafs,
				(int)game_state->world.bsp_geometry.leafs.size());

			ImGui::Text("Drawn faces: %6d / %6d",
				debug.drawn_faces,
				(int)faces.size());
			ImGui::Text("Snapshot ents  : %6u", game_state->snapshot_entity_count);
			ImGui::Text("Traversed nodes: %6d", debug.traversed_nodes);
			ImGui::Text("Current leaf   : %6d", game_state->world.cur_leaf[0].leaf_id);
			ImGui::Text("FPS            : %6.2f", fps);
			ImGui::Text("Camera         : %4.2f %4.2f %4.2f",
				camera.origin.x,
				camera.origin.y,
				camera.origin.z);

			const auto& snapshot = game_state->latest_snapshot;

			for (uint32_t i = 0; i < snapshot.entity_count && i < 4; ++i)
			{
				const auto& e = snapshot.entities[i];

				ImGui::Text
				(
					"ent %u: id=%u pos=(%4.4f,%4.4f,%4.4f)",
					i, e.entity_id,
					e.origin[0], e.origin[1], e.origin[2]
				);
			}


			const char* s = nullptr;

			using enum protocol::client_state;
			switch(game_state->connection_state)
			{
			case disconnected:
				s = "disconnected";
				break;
			case waiting_conn_ack:
				s = "waiting_conn_ack";
				break;
			case checking_files:
				s = "checking_files";
				break;
			case waiting_files_ack:
				s = "waiting_files_ack";
				break;
			case downloading_file:
				s = "downloading_file";
				break;
			case waiting_file_fragment:
				s = "waiting_file_fragment";
				break;
			case waiting_file_ack:
				s = "waiting_file_ack";
				break;
			case loading_resources:
				s = "loading_resources";
				break;
			case ready:
				s = "ready";
				break;
			case in_game:
				s = "in_game";
				break;
			case disconnecting:
				s = "disconnecting";
				break;
			case error:
				s = "error";
				break;
			default:
				s = "unexcepted";
				break;
			}
			ImGui::Text("stage: %s", s);

		}
		ImGui::End();
	}


	
	void Renderer::render_UI_boot()
	{
	}

	
	void Renderer::render_UI_main_menu_disconnected()
	{
		if (ImGui::Button("SinglePlayer"))
		{
			game_state->ui_cmd = client::UICommand::singleplayer;
		}
		if (ImGui::Button("MultiPlayer"))
		{
			game_state->ui_cmd = client::UICommand::multiplayer;
		}
	}
    void Renderer::render_UI_main_menu_starting_local_server()
	{
		ImGui::Text("starting_local_server");
		if (ImGui::Button("cancel"))
		{
		}
	}
    void Renderer::render_UI_main_menu_connecting()
	{
		ImGui::Text("connecting");
		
		if (ImGui::Button("cancel"))
		{
			game_state->ui_cmd = client::UICommand::cancel;
		}
	}

    void Renderer::render_UI_main_menu_loading()
	{
		ImGui::Text("loading");
		if (ImGui::Button("cancel"))
		{
		}
	}
    void Renderer::render_UI_main_menu_disconnecting()
	{
		ImGui::Text("disconnecting");
		if (ImGui::Button("cancel"))
		{
		}
	}
    void Renderer::render_UI_main_menu_error()
	{
		ImGui::Text("error");
		if (ImGui::Button("ok"))
		{
		}
	}

	void Renderer::render_UI_main_menu()
	{
		using enum client::SessionStage;
		switch (game_state->session_stage) 
		{
		case disconnected:
			render_UI_main_menu_disconnected();
			break;
		case starting_local_server:
			render_UI_main_menu_starting_local_server();
			break;
		case connecting:
			render_UI_main_menu_connecting();
			break;
		case loading:
			render_UI_main_menu_loading();
			break;
		/* 
		case in_game: // in main_menu state it can't be SessionStage::in_game
			break;
		*/
		case disconnecting:
			render_UI_main_menu_disconnecting();
			break;
		case error:
			render_UI_main_menu_error();
			break;
		default:
			break;
		}

		
	}

	void Renderer::render_UI_in_game()
	{		
		render_UI_debug();
		if (game_state->menu)
		{
			if (ImGui::Button("Disconnect"))
			{
				game_state->ui_cmd = client::UICommand::disconnect;
			}
		}
	}

	void Renderer::render_UI_shutdown()
	{
	}


	void Renderer::render_UI()
	{

		if (!initialized)
			return;

		render_UI_start();


		ImGui::SetNextWindowBgAlpha(0.0f);
		
		render_UI_debug();
		
		ImGui::SetNextWindowPos(ImVec2(380, 250), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(260, 180), ImGuiCond_Always);

		if (ImGui::Begin("Tungsten", nullptr,
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse))
		{
			using enum client::MainStage;
			switch (game_state->main_stage)
			{
			case boot:
				render_UI_boot();
				break;
			case main_menu:
				render_UI_main_menu();
				break;
			case in_game:
				render_UI_in_game();
				break;
			case shutdown:
				render_UI_shutdown();
				break;
			default:
				break;
			}
		}
		ImGui::End();
		

		render_UI_end();
	}


	void Renderer::render_secondary()
	{
		render_HUD();
		render_UI();
	}

	void Renderer::render()
	{
		if (!initialized || game_state == nullptr)
			return;

		render_start();

		render_main();
		render_secondary();
		
		render_end();
	}
}
