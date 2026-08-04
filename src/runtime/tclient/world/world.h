#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>
#include <cstdint>
#include <string>

#include "../../../common/formats/tbsp/tbsp.h"


namespace tungsten::world
{
    struct cl_plane
    {
        glm::vec3 normal{ 0.0f };
        float dist = 0.0f;

        enum cl_plane_type : uint8_t
        {
            ARBITRARY = 0,
            X,
            Y,
            Z,
            XY,
            XZ,
            YZ,
        };
        
        cl_plane_type type = ARBITRARY;

        float distTo(const glm::vec3& p) const;

        void updateType();
    };

    struct cl_node
    {
        int plane_id = -1;
        int children[2]{ -1, -1 }; // >= 0 node index, < 0 encoded leaf index

        glm::vec3 mins{ 0.0f };
        glm::vec3 maxs{ 0.0f };
    };

    struct cl_leaf_range
    {
        int first = -1;
        int num = -1;
    };

    struct cl_leaf
    {
        int contents = CONTENTS_EMPTY;

        cl_leaf_range face_range;
        cl_leaf_range portal_range;

        glm::vec3 mins{ 0.0f };
        glm::vec3 maxs{ 0.0f };
    };


    struct cl_bsp_geometry
    {
        std::vector<cl_plane>  planes{};
        std::vector<cl_node>   nodes{};
        std::vector<cl_leaf>   leafs{};

        int headnodes[MAX_MAP_HULLS] = { -1, -1, -1, -1 };

        int findLeaf(int headnode, const glm::vec3& p) const;
        void clear();
    };

    struct cl_precompile_bitset
    {
        int leaf_count = 0;
        std::vector<uint8_t> pvs_data{}; // pvs_data.size() == leaf_count * leaf_count

        bool leafInSet(int from_leaf, int test_leaf) const;
    };


    struct cl_entity
    {
        int         id = -1;
        std::string name {};
        
        glm::vec3 origin{ 0.0 };
        glm::quat rotation = glm::identity<glm::quat>();
    };

    struct cl_world
    {
        cl_bsp_geometry      bsp_geometry{};
    
        cl_precompile_bitset vis {};
        cl_precompile_bitset hear{};

        std::vector<std::string> classname_table{};
        std::vector<cl_entity>   entities       {};

        struct cur_leaf_pair
        {
            int leaf_id         = -1;
            const cl_leaf* leaf = nullptr;
        } cur_leaf[MAX_MAP_HULLS]{};

        void updateCurLeaf(glm::vec3 origin);
        bool parseBSPMap(const BSPMap& bspmap);
    };
}