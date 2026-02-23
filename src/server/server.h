#pragma once


class Server
{
public:
	Server() = default;
	Server(const Server&) = delete;
	Server(Server&&) = delete;
	virtual ~Server() = default;


};