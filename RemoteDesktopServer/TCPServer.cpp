#include "stdafx.h"
#include "common.h"

TCPServer::TCPServer(const char *nodename, const char *servname)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;
	
	if (getaddrinfo(nodename, servname, &hints, &srvinfo) != 0) {
		fail("getaddrinfo() failed.");
	}

	srvsocket = socket(srvinfo->ai_family, srvinfo->ai_socktype, srvinfo->ai_protocol);
    if (srvsocket == INVALID_SOCKET) {
		fail("socket() failed.");
	}

	/*BOOL flag = TRUE;
	int r = setsockopt(srvsocket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(BOOL));
	assert(r == 0);*/
}

TCPServer::~TCPServer()
{
	closesocket(srvsocket);
	freeaddrinfo(srvinfo);
}

void TCPServer::DoModal()
{
	if (bind(srvsocket, srvinfo->ai_addr, (int) srvinfo->ai_addrlen) == SOCKET_ERROR) {
		fail("bind() error.");
	}
	if (listen(srvsocket, SOMAXCONN) == SOCKET_ERROR) {
		fail("listen() error.");
	}

	pmsg("server address is %s:%d\n",
		inet_ntoa(((struct sockaddr_in *) srvinfo->ai_addr)->sin_addr),
		(int) ntohs(((struct sockaddr_in *) srvinfo->ai_addr)->sin_port));

	struct sockaddr_in cliaddr;
	while (1) {
		int cliaddr_size = sizeof(cliaddr);
		SOCKET clisocket = accept(srvsocket, (struct sockaddr *) &cliaddr, &cliaddr_size);
		if (clisocket == INVALID_SOCKET) {
			return;
		}
		pmsg("accepted from %s:%d\n", inet_ntoa(cliaddr.sin_addr), (int) ntohs(cliaddr.sin_port));
		
		TCPConnection *ConnInst = new TCPConnection(clisocket);
		ConnInst->DoModal();
		delete ConnInst;
	}
}