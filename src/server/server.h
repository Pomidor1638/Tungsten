
#ifndef MEGAGAME_SERVER_H
#define MEGAGAME_SERVER_H

#include <glm/glm.hpp>


using sv_real = float;
using sv_vec3 = glm::vec3;
using sv_vec2 = glm::vec2;

struct cl_snapshot
{
	sv_vec3 wishdir;
	uint64_t key_state;
};

struct sv_snapshot
{
	sv_vec3 origin;
};


class Server
{
public:

	Server();
	Server(const Server&) = delete;
	Server(Server&&) = delete;

	virtual ~Server();
	
	bool init();
	void shutdown();

	void tick(uint64_t delta_time);

	void processClientSnapshot(cl_snapshot cl_snp);
	sv_snapshot getSnapshot() const;

private:

	bool is_active = false;
	glm::vec3 player_pos{ 0 };

	uint64_t current_time = 0;
	sv_real frac_delta    = 0.0;

};

#endif