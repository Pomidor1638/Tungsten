
#include "server.h"


Server::Server()
	: is_active{false}
	, player_pos{ 0 }
{

}

Server::~Server()
{

}

bool Server::init()
{
	return true;
}
void Server::shutdown()
{
}

void Server::tick(uint64_t delta_time)
{
	frac_delta = delta_time / 1000.0;
}

void Server::processClientSnapshot(cl_snapshot cl_snp)
{
	player_pos += cl_snp.wishdir * frac_delta;
}
sv_snapshot Server::getSnapshot() const
{
	return sv_snapshot{ player_pos };
}
