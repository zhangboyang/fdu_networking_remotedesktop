#include "stdafx.h"
#include "common.h"

int main()
{
	pmsg("welcome to remote desktop server.\n");

	// increase timer resolution
	UINT timer_res = 1;
	timeBeginPeriod(timer_res);

	// initial WSA socket
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fail("can't initialize WSA.");
	}
	
	// start the TCP server
	TCPServer *SrvInst = new TCPServer(NULL, "1223");
	SrvInst->DoModal();
	delete SrvInst;
	
	// cleanup WSA socket
	WSACleanup();
	
	// cleanup timer resolution
	timeEndPeriod(timer_res);

	return 0;
}