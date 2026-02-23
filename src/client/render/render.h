//
// Created by UBER_USER on 29.12.2025.
//

#ifndef MEGAGAME_RENDER_H
#define MEGAGAME_RENDER_H

#include <SDL.h>
#include <glm/glm.hpp>
#include "../../common/common.h"
#include <vector>

struct R_Plane
{
    glm::vec3 normal;
    float dist;
};

struct DrawRange
{
    uint32_t texture_id;
    uint32_t start_index;
    uint32_t count;
    bool     is_portal;
};

struct RenderVertex
{
    glm::vec3 position;
    glm::vec2 uv;
};

struct R_Leaf 
{
    glm::vec3 mins, maxs;
    int contents;
    std::vector<DrawRange> polygons;
    std::vector<DrawRange> portals;
};

struct R_Node 
{
    glm::vec3 mins, maxs;
    R_Plane plane;

    int32_t children[2];
};

struct RenderMap 
{

    struct Buffer_Context
    {
        uint32_t vao = 0;
        uint32_t vbo = 0;
        uint32_t ebo = 0;
    };

    Buffer_Context polygons;
    Buffer_Context portals;

    std::vector<R_Node> nodes;
    std::vector<R_Leaf> leafs;
    int32_t headnode = 0;

    bool valid = false;

};


class Render
{
public:

    void processEvent(const SDL_Event& e);
    void startProcessEvent();
    void endProcessEvent();

    void update(double delta_time);

    void renderStart();
    void renderUI();
    void renderHUD();
    void renderWorld();
    void renderEntities();
    void renderEnd();

    void setCamOrigin(glm::vec3 origin);
    void setCamAngles(glm::vec3 angles);

    glm::vec3 getCamOrigin() const;
    glm::vec3 getCamAngles() const;

    void loadBSPMap(const BSPMap& bspmap);

    using LoadMapCallback = std::function<void()>;

    void setLoadMapCallback(LoadMapCallback cb) { loadMapCb = cb; }


private:

    void uploadBSPTextures(const BSPMap& bsp);
    void updateLeafNum();
    void setPerspective();

    void drawLeafPolygons(const R_Leaf& leaf, uint32_t& last_tid);
    void drawLeafPortals(const R_Leaf& leaf);

    SDL_GLContext gl_context = nullptr;
    SDL_Window* window = nullptr;

    RenderMap* map = nullptr;

    struct Camera
    {    
        glm::vec3 origin;
        glm::vec3 angles;
    } cur_cam{};


    std::vector<uint32_t> textures;

    double delta_time = 0.0;

    int leafnum = 0;

    bool draw_polygons            = true;
    bool draw_polygons_in_curleaf = false;
    bool draw_portals             = false;
    bool draw_portals_in_curleaf  = false;

    LoadMapCallback loadMapCb = nullptr;

public:

    Render(SDL_Window* w);
    virtual ~Render();

    Render(const Render&) = delete;
    Render(Render&&) = delete;
};


#endif //MEGAGAME_RENDER_H