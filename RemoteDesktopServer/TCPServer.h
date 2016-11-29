#pragma once

class TCPServer {
private:
	SOCKET srvsocket;
	struct addrinfo *srvinfo;
public:
	TCPServer(const char* nodename, const char* servname);
	~TCPServer();
	void DoModal();
};

