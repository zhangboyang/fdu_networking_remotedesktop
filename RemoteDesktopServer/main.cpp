#include "stdafx.h"
#include "common.h"

char rdpsw[MAXLINE];

int main()
{
	int cfgflag = 0;
	char port[MAXLINE];
	FILE *fp = fopen("server.txt", "r");
	if (fp) {
		if (!fgets(port, sizeof(port), fp)) goto closecfg;
		if (!fgets(rdpsw, sizeof(rdpsw), fp)) goto closecfg;
		if (strrchr(port, '\n')) *strrchr(port, '\n') = '\0';
		if (strrchr(rdpsw, '\n')) *strrchr(rdpsw, '\n') = '\0';
		cfgflag = 1;
closecfg:
		fclose(fp);
	}
	if (!cfgflag) {
		strcpy(port, "1223");
		strcpy(rdpsw, "123456");
		pmsg("error during reading config file, using default configration.\n");
	}

	pmsg("welcome to remote desktop server.\n");
	pmsg("password is \"%s\"\n", rdpsw);

	// increase timer resolution
	UINT timer_res = 1;
	timeBeginPeriod(timer_res);

	// initial WSA socket
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fail("can't initialize WSA.");
	}
	
	// start the TCP server
	TCPServer *SrvInst = new TCPServer(NULL, port);
	SrvInst->DoModal();
	delete SrvInst;
	
	// cleanup WSA socket
	WSACleanup();
	
	// cleanup timer resolution
	timeEndPeriod(timer_res);

	return 0;
}