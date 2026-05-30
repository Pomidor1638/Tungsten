
#include "world.h"

namespace tungsten::world
{
	float cl_plane::distTo(const glm::vec3& p) const
	{
		switch (type)
		{
		case X:
		case Y:
		case Z:
		{
			int t = type - 1;
			return p[t] - dist;
		}
		case ARBITRARY:
		case XY:
		case XZ:
		case YZ:
		default:
			break;
		}

		return glm::dot(normal, p) - dist;
	}

	void cl_plane::updateType()
	{
		for (int i = 0; i < 3; i++)
		{
			if (normal[i] == 1.0f)
			{
				type = cl_plane_type(i + 1);
				return;
			}
		}

		for (int i = 0; i < 3; i++)
		{
			if (normal[i] == 0.0f)
			{
				type = cl_plane_type(YZ - i);
				return;
			}
		}

		type = ARBITRARY;
	}


	void cl_bsp_geometry::clear()
	{

		planes.clear();
		nodes.clear();
		leafs.clear();

		for (auto& x : headnodes)
			x = -1;
	}

	int cl_bsp_geometry::findLeaf(int headnode, const glm::vec3& p) const
	{
		if (headnode < 0)
			return -1;

		while (headnode >= 0)
		{
			const auto& node  = nodes [headnode];
			const auto& plane = planes[node.plane_id];

			// if distTo() >= 0 it's front and t == 0
			// if distTo() <  0 it's back  and t == 1

			const auto t = plane.distTo(p) < 0;
			headnode     = node.children[t];

			if (headnode >= leafs.size())
				return -1;
		}

		return ~headnode;
	}

	bool cl_precompile_bitset::leafInSet(int from_leaf, int test_leaf) const
	{
		return pvs_data[from_leaf * leaf_count + test_leaf];
	}

	glm::vec3 dvec3ToVec3(dvec3_t dv)
	{
		return { dv.v[0], dv.v[1], dv.v[2] };
	}

	cl_plane dplaneToCL_PLane(dplane_t dplane)
	{
		cl_plane cplane;
		cplane.normal = dvec3ToVec3(dplane.normal);
		cplane.dist      = dplane.dist;
		cplane.updateType();

		return cplane;
	}

	bool makePlanes(std::vector<cl_plane>& planes, const BSPMap& bspmap)
	{
		size_t n = bspmap.planes.size();

		if (n > MAX_MAP_PLANES)
		{
			planes.clear();
			return false;
		}

		planes.resize(n);
		for (size_t i = 0; i < n; i++)
		{
			planes[i] = dplaneToCL_PLane(bspmap.planes[i]);
		}

		return true;
	}

	cl_node dnodeToCL_Node(dnode_t dnode)
	{
		cl_node cnode;

		cnode.plane_id = dnode.planenum;
		cnode.children[0] = dnode.children[0];
		cnode.children[1] = dnode.children[1];

		cnode.mins = dvec3ToVec3(dnode.mins);
		cnode.maxs = dvec3ToVec3(dnode.maxs);
			
		return cnode;
	}

	bool makeNodes(std::vector<cl_node>& nodes, const BSPMap& bspmap)
	{
		size_t n = bspmap.nodes.size();

		if (n > MAX_MAP_NODES)
		{
			nodes.clear();
			return false;
		}

		nodes.resize(n);
		for (size_t i = 0; i < n; i++)
		{
			nodes[i] = dnodeToCL_Node(bspmap.nodes[i]);
		}


		return true;
	}

	cl_leaf dleafToCL_Leaf(dleaf_t dleaf)
	{
		cl_leaf cleaf;

		cleaf.contents           = dleaf.contents;

		cleaf.face_range.first   = dleaf.firstface;
		cleaf.face_range.num     = dleaf.numfaces;

		cleaf.portal_range.first = dleaf.firstportal;
		cleaf.portal_range.num   = dleaf.numportals;

		cleaf.mins = dvec3ToVec3(dleaf.mins);
		cleaf.maxs = dvec3ToVec3(dleaf.maxs);

		return cleaf;
	}

	bool makeLeafs(std::vector<cl_leaf>& leafs, const BSPMap& bspmap)
	{
		size_t n = bspmap.leafs.size();

		if (n > MAX_MAP_LEAFS)
		{
			leafs.clear();
			return false;
		}
		
		leafs.resize(n);
		for (size_t i = 0; i < n; i++)
		{
			leafs[i] = dleafToCL_Leaf(bspmap.leafs[i]);
		}

		return true;
	}


	void cl_world::updateCurLeaf(glm::vec3 origin)
	{
		const auto& s = bsp_geometry;
		for (int i = 0; i < MAX_MAP_HULLS; i++)
		{
			auto& node_id = cur_leaf[i].leaf_id;
			node_id       = s.headnodes[i];

			while (node_id >= 0)
			{
				const auto& node = s.nodes[node_id];
				const auto& plane = s.planes[node.plane_id];
				bool t = plane.distTo(origin) < 0; // because >= 0 is 1, it means oposite 
				node_id = node.children[t];
			}
			node_id = ~node_id; // -(i + 1)
			cur_leaf[i].leaf = &s.leafs[node_id];
		}
	}

	bool cl_world::parseBSPMap(const BSPMap& bspmap)
	{
			auto& s = bsp_geometry;

			s.clear();

			if (!makePlanes(s.planes, bspmap)) return false;
			if (!makeNodes (s.nodes , bspmap)) return false;
			if (!makeLeafs (s.leafs , bspmap)) return false;

			s.headnodes[0] = bspmap.models[0].headnode[0];

			const int headnode = s.headnodes[0];

			if (headnode < 0 || static_cast<size_t>(headnode) >= s.nodes.size())
				return false;

		return true;
	}
}

