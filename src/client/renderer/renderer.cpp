#include "renderer.h"

#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_sdl2.h"
#include "../../imgui/imgui_impl_opengl3.h"

Renderer::Renderer(const GameState* s, SDL_Window* w)
	: initialized{ false }
	, window{ w }
	, game_state{ s }
{
	if (!window || !game_state)
	{
	}
	else
	{
		initialized = true;
	}
}

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
		return false;

	if (!ImGui_ImplOpenGL3_Init("#version 130"))
		return false;

	return true;
}

bool Renderer::init()
{
	if (!window)
	{
		return false;
	}

	gl_context = SDL_GL_CreateContext(window);
	if (!gl_context)
	{
		return false;
	}


	SDL_GL_MakeCurrent(window, gl_context);

	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
	{
		return false;
	}

	if (!InitImGui(window, gl_context))
	{
		return false;
	}

	SDL_GL_SetSwapInterval(0);

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
{
}
void Renderer::processEvent(const SDL_Event& event)
{
	ImGui_ImplSDL2_ProcessEvent(&event);
}
void Renderer::endProcessEvent()
{
}


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
	SDL_GetWindowSize(window, &width, &height);

	camera.aspect = static_cast<float>(width) / static_cast<float>(height);
	camera.updateMatrices();
	camera.updateFrustumPoints();

	const auto& world = game_state->world;
	const auto& cur_leaf = world.cur_leaf[0].leaf;

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
	const auto& world = game_state->world;
	const auto& s = world.bsp_geometry;
	const auto& leaf = s.leafs[leaf_id];
	const auto& f = leaf.face_range;

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
		node_id = ~node_id;
		if (!node_id)
			return;

		const auto& leaf = s.leafs[node_id];
		const auto& cur_leaf = world.cur_leaf[0].leaf;

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
		{
		}
		else
		{
			render_leaf(node_id);
		}
		return;
	}

	int counts[3] = { 0, 0, 0 };

	const auto& frustum_points = camera.frustum_points;
	const auto& node           = s.nodes[node_id];
	const auto& plane          = s.planes[node.plane_id];

	for (int i = 0; i < 8; i++)
	{
		float t = plane.distTo(frustum_points[i]);

		if      (t > 0) counts[0]++;
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
	glBindVertexArray(faces_buffer.vao);
	bsp_traverse(game_state->world.bsp_geometry.headnodes[0]);
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
{

}

void Renderer::render_main()
{
	if (game_state->global_state != GlobalGameState::IN_GAME)
		return;

	render_main_start();

	render_world();
	
	render_main_end();

}


void Renderer::render_HUD_start()
{
}

void Renderer::render_HUD_end()
{
}

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
}
void Renderer::render_UI_end()
{

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::render_UI()
{
	render_UI_start();

	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(250, 120), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.35f);

	const float dt = ImGui::GetIO().DeltaTime;
	const float fps = (dt > 0.0f) ? (1.0f / dt) : 0.0f;

	if (ImGui::Begin("Debug render", nullptr,
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoNav))
	{
		ImGui::Text("Drawn leafs: %6d / %6d",
			debug.drawn_leafs,
			(int)game_state->world.bsp_geometry.leafs.size());

		ImGui::Text("Drawn faces: %6d / %6d",
			debug.drawn_faces,
			(int)faces.size());

		ImGui::Text("Traversed nodes: %6d", debug.traversed_nodes);
		ImGui::Text("Current leaf   : %6d", game_state->world.cur_leaf[0].leaf_id);
		ImGui::Text("FPS            : %6.2f", fps);
		ImGui::Text("Camera         : %4.2f %4.2f %4.2f",
			camera.origin.x,
			camera.origin.y,
			camera.origin.z);
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