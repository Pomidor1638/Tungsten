#include "render.h"
#include <stdexcept>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include "../../imgui/imgui.h"
#include "../../imgui/imgui_impl_sdl2.h"
#include "../../imgui/imgui_impl_opengl3.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>




Render::Render(SDL_Window* w)
    : window(w)
{
    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
    {
        throw std::runtime_error(std::string(__FUNCSIG__) + ": can't create GL context");
    }
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        throw std::runtime_error(std::string(__FUNCSIG__) + ": failed to initialize GLAD");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    SDL_GL_SetSwapInterval(0);

    map = hunk.allocate<RenderMap, 1>();
}


void Render::uploadBSPTextures(const BSPMap& bsp) 
{
    if (bsp.texture_block.empty()) 
        return;

    if (!textures.empty()) 
    {
        glDeleteTextures((GLsizei)textures.size(), textures.data());
        textures.clear();
    }

    const byte* base_ptr = bsp.texture_block.data();
    const dtexturelump_t* lump = reinterpret_cast<const dtexturelump_t*>(base_ptr);

    for (int i = 0; i < lump->numtex; i++) 
    {
        int offset = lump->dataofs[i];
        if (offset == -1) 
        {
            textures.push_back(0);
            continue;
        }

        const dtexture_t* mip = reinterpret_cast<const dtexture_t*>(base_ptr + offset);
        const byte* pixel_data = reinterpret_cast<const byte*>(mip) + sizeof(dtexture_t);

        uint32_t tid;
        glGenTextures(1, &tid);
        glBindTexture(GL_TEXTURE_2D, tid);

        GLenum format = (mip->colortype == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, format, mip->width, mip->height, 0, format, GL_UNSIGNED_BYTE, pixel_data);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        textures.push_back(tid);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}


void Render::loadBSPMap(const BSPMap& bsp)
{
    map->valid = false;
    uploadBSPTextures(bsp);

    std::vector<RenderVertex> world_verts;
    std::vector<uint32_t>     world_indices;

    std::vector<glm::vec3>    portal_verts;
    std::vector<uint32_t>     portal_indices;

    map->leafs.resize(bsp.leafs.size());
    map->nodes.resize(bsp.nodes.size());
    map->headnode = bsp.models[0].headnode[0];

    const byte* tex_base = bsp.texture_block.data();
    const dtexturelump_t* tex_lump = reinterpret_cast<const dtexturelump_t*>(tex_base);

    for (size_t i = 0; i < bsp.nodes.size(); i++) 
    {
        const auto& dn = bsp.nodes[i];
        const dplane_t& dp = bsp.planes[dn.planenum];
        R_Node& node = map->nodes[i];
        node.plane.dist = dp.dist;
        node.plane.normal = glm::vec3(dp.normal.v[0], dp.normal.v[1], dp.normal.v[2]);
        node.mins = glm::vec3(dn.mins.v[0], dn.mins.v[1], dn.mins.v[2]);
        node.maxs = glm::vec3(dn.maxs.v[0], dn.maxs.v[1], dn.maxs.v[2]);
        node.children[0] = dn.children[0];
        node.children[1] = dn.children[1];
    }

    for (size_t i = 0; i < bsp.leafs.size(); i++)
    {
        const dleaf_t& bl = bsp.leafs[i];
        R_Leaf& out_leaf = map->leafs[i];
        out_leaf.contents = bl.contents;
        out_leaf.mins = glm::vec3(bl.mins.v[0], bl.mins.v[1], bl.mins.v[2]);
        out_leaf.maxs = glm::vec3(bl.maxs.v[0], bl.maxs.v[1], bl.maxs.v[2]);


        for (uint32_t j = 0; j < bl.numfaces; j++)
        {
            const dface_t& f = bsp.faces[bl.firstface + j];
            const dtexinfo_t& ti = bsp.texture_infos[f.texinfonum];

            float tw = 1.0f, th = 1.0f;
            if (ti.texnum >= 0 && (size_t)ti.texnum < textures.size()) 
            {
                const dtexture_t* m = reinterpret_cast<const dtexture_t*>(tex_base + tex_lump->dataofs[ti.texnum]);
                tw = (float)m->width; th = (float)m->height;
            }

            DrawRange dr;
            dr.start_index = (uint32_t)world_indices.size();
            dr.texture_id = (ti.texnum >= 0) ? textures[ti.texnum] : 0;

            uint32_t base_v = (uint32_t)world_verts.size();
            for (int v = 0; v < f.numpoints; v++) 
            {
                uint32_t v_idx = bsp.vertex_indexes[f.firstpoint + v];
                glm::vec3 pos = glm::vec3(bsp.vertexes[v_idx].v[0], bsp.vertexes[v_idx].v[1], bsp.vertexes[v_idx].v[2]);

                RenderVertex rv;
                rv.position = pos;
                rv.uv.x = (glm::dot(pos, glm::vec3(ti.vecs[0][0], ti.vecs[0][1], ti.vecs[0][2])) + ti.vecs[0][3]) / tw;
                rv.uv.y = (glm::dot(pos, glm::vec3(ti.vecs[1][0], ti.vecs[1][1], ti.vecs[1][2])) + ti.vecs[1][3]) / th;
                world_verts.push_back(rv);

                if (v >= 2) {
                    world_indices.push_back(base_v);
                    world_indices.push_back(base_v + v - 1);
                    world_indices.push_back(base_v + v);
                }
            }

            dr.count = (uint32_t)world_indices.size() - dr.start_index;
            out_leaf.polygons.push_back(dr);
        }

        // --- ПОРТАЛЫ ---
        for (uint32_t j = 0; j < bl.numportals; j++)
        {
            uint32_t p_idx = bsp.portal_indexes[bl.firstportal + j];
            const dportal_t& port = bsp.portals[p_idx];

            DrawRange pr;
            pr.start_index = (uint32_t)portal_indices.size();
            pr.count = port.numpoints;
            pr.is_portal = true;

            uint32_t base_v = (uint32_t)portal_verts.size();
            for (int v = 0; v < port.numpoints; v++) {
                uint32_t v_idx = bsp.vertex_indexes[port.firstpoint + v];
                portal_verts.push_back(glm::vec3(bsp.vertexes[v_idx].v[0], bsp.vertexes[v_idx].v[1], bsp.vertexes[v_idx].v[2]));

                portal_indices.push_back(base_v + v);
            }
            out_leaf.portals.push_back(pr);
        }

    }

    // 4. Загрузка в GPU (Используем Buffer_Context)

    // 4.1 Полигоны (RenderVertex: Pos + UV)
    glGenVertexArrays(1, &map->polygons.vao);
    glBindVertexArray(map->polygons.vao);

    glGenBuffers(1, &map->polygons.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, map->polygons.vbo);
    glBufferData(GL_ARRAY_BUFFER, world_verts.size() * sizeof(RenderVertex), world_verts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &map->polygons.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, map->polygons.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, world_indices.size() * sizeof(uint32_t), world_indices.data(), GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(RenderVertex), (void*)offsetof(RenderVertex, position));
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, sizeof(RenderVertex), (void*)offsetof(RenderVertex, uv));

    // 4.2 Порталы (Только vec3: Pos)
    glGenVertexArrays(1, &map->portals.vao);
    glBindVertexArray(map->portals.vao);

    glGenBuffers(1, &map->portals.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, map->portals.vbo);
    glBufferData(GL_ARRAY_BUFFER, portal_verts.size() * sizeof(glm::vec3), portal_verts.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &map->portals.ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, map->portals.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, portal_indices.size() * sizeof(uint32_t), portal_indices.data(), GL_STATIC_DRAW);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(glm::vec3), (void*)0);

    glBindVertexArray(0);

    map->valid = true;
}







Render::~Render()
{
    if (!textures.empty())
    {
        glDeleteTextures(static_cast<GLsizei>(textures.size()), textures.data());
        textures.clear();
    }

    if (map)
    {
        if (map->polygons.vao) glDeleteVertexArrays(1, &map->polygons.vao);
        if (map->polygons.vbo) glDeleteBuffers(1, &map->polygons.vbo);
        if (map->polygons.ebo) glDeleteBuffers(1, &map->polygons.ebo);

        if (map->portals.vao) glDeleteVertexArrays(1, &map->portals.vao);
        if (map->portals.vbo) glDeleteBuffers(1, &map->portals.vbo);
        if (map->portals.ebo) glDeleteBuffers(1, &map->portals.ebo);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (gl_context)
    {
        SDL_GL_DeleteContext(gl_context);
    }
}



void Render::startProcessEvent()
{
    updateLeafNum();
}
void Render::endProcessEvent()
{
}

void Render::processEvent(const SDL_Event& e)
{
    ImGui_ImplSDL2_ProcessEvent(&e);
}

void Render::setPerspective()
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    float aspect = (float)w / (float)h;
    float fov   = 60.0f;
    float zNear = 1.0f;
    float zFar  = 20000.0f;
    float fH    = tan(fov / 360.0f * M_PI) * zNear;
    float fW    = fH * aspect;

    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}

void Render::drawLeafPolygons(const R_Leaf& leaf, uint32_t& last_tid)
{
    for (const auto& range : leaf.polygons)
    {
        if (range.count == 0) continue;

        if (range.texture_id != last_tid)
        {
            glBindTexture(GL_TEXTURE_2D, range.texture_id);
            last_tid = range.texture_id;
        }

        glDrawElements(GL_TRIANGLES, range.count, GL_UNSIGNED_INT,
            (void*)(uintptr_t)(range.start_index * sizeof(uint32_t)));
    }
}

void Render::drawLeafPortals(const R_Leaf& leaf)
{
    for (const auto& pr : leaf.portals)
    {
        if (pr.count == 0) continue;

        glDrawElements(GL_LINE_LOOP, pr.count, GL_UNSIGNED_INT,
            (void*)(uintptr_t)(pr.start_index * sizeof(uint32_t)));
    }
}

void Render::renderWorld()
{
    if (!map || map->polygons.vao == 0)
        return;

    setPerspective();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRotatef(-cur_cam.angles.x, 1, 0, 0);
    glRotatef(-cur_cam.angles.y, 0, 1, 0);
    glRotatef(-cur_cam.angles.z, 0, 0, 1);
    glTranslatef(-cur_cam.origin.x, -cur_cam.origin.y, -cur_cam.origin.z);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    uint32_t last_tid = 0xFFFFFFFF;

    glBindVertexArray(map->polygons.vao);

    if (draw_polygons)
    {
        if (draw_polygons_in_curleaf && leafnum > 0 && leafnum < (int)map->leafs.size())
        {
            drawLeafPolygons(map->leafs[leafnum], last_tid);
        }
        else
        {
            for (const auto& leaf : map->leafs)
            {
                if (leaf.contents == CONTENTS_SOLID)
                    continue;
                drawLeafPolygons(leaf, last_tid);
            }
        }
    }

    if (draw_portals && map->portals.vao != 0)
    {
        glBindVertexArray(map->portals.vao);
        glDisable(GL_TEXTURE_2D);
        glLineWidth(2.0f);
        glColor4f(0.0f, 1.0f, 1.0f, 1.0f);

        if (draw_portals_in_curleaf)
        {
            if (leafnum > 0 && leafnum < (int)map->leafs.size())
                drawLeafPortals(map->leafs[leafnum]);
        }
        else
        {
            for (const auto& leaf : map->leafs)
            {
                if (leaf.contents == CONTENTS_SOLID)
                    continue;
                drawLeafPortals(leaf);
            }
        }
    }

    //glBindVertexArray(0);
}

void Render::renderEntities() 
{
}
void Render::renderHUD() 
{
}

void Render::renderUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Debug"))
    {
        float fps = static_cast<float>(1.0 / delta_time);
        float ms = static_cast<float>(delta_time * 1000.0);
        ImGui::Text("Performance: %.1f FPS (%.3f ms)", fps, ms);

        ImGui::Separator();

        if (ImGui::CollapsingHeader("Camera Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("Position (X, Y, Z)");
            if (ImGui::DragFloat3("##origin", &cur_cam.origin.x))
            {
            }

            ImGui::Text("Rotation (Pitch, Roll, Yaw)");
            if (ImGui::SliderFloat3("##angles", &cur_cam.angles.x, 0, 360.0f))
            {
            }

            if (ImGui::Button("Reset Camera"))
            {
                cur_cam.origin = glm::vec3(0, 0, 0);
                cur_cam.angles = glm::vec3(0, 0, 0);
            }

            ImGui::Separator();
        }

        ImGui::Text("Current Camera Origin: %.2f, %.2f, %.2f", cur_cam.origin.x, cur_cam.origin.y, cur_cam.origin.z);

        ImGui::Text("Current leafnum: %i", leafnum);

        ImGui::Checkbox("Draw polygons", &draw_polygons);
        if (draw_polygons)
        {
            ImGui::SameLine();
            ImGui::Checkbox("Draw polygons in curleaf", &draw_polygons_in_curleaf);
        }

        ImGui::Checkbox("Draw portals", &draw_portals);
        if (draw_portals)
        {
            ImGui::SameLine();
            ImGui::Checkbox("Draw portals in curleaf", &draw_portals_in_curleaf);
        }
    
        ImGui::Separator();
        ImGui::Text("Map Management");
        if (ImGui::Button("Load Map")) 
        {
            if (loadMapCb) 
            {
                loadMapCb(); // Вызываем функцию, которую передал Client
            }
        }

    }
    ImGui::End();




    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}





void Render::renderStart()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Render::renderEnd()
{
    SDL_GL_SwapWindow(window);
}


void Render::setCamOrigin(glm::vec3 origin)
{
    cur_cam.origin = origin;
}
void Render::setCamAngles(glm::vec3 angles)
{
    cur_cam.angles = angles;
}

glm::vec3 Render::getCamOrigin() const
{
    return cur_cam.origin;
}
glm::vec3 Render::getCamAngles() const
{
    return cur_cam.angles;
}

void Render::update(double delta_time)
{
    this->delta_time = delta_time;
}

void Render::updateLeafNum() 
{
    if (!map || !map->valid)
        return;
    int i = map->headnode;
    while (i >= 0)
    {
        const auto& node = map->nodes[i];
        const auto& plane = node.plane;
        float t = glm::dot(plane.normal, cur_cam.origin) - plane.dist;
        int side = t >= 0 ? 0 : 1;
        i = node.children[side];
    }
    leafnum = ~i; // -(i + 1)
}
