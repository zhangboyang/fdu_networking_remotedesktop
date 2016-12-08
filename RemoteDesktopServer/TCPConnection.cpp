#include "stdafx.h"
#include "common.h"

TCPConnection::TCPConnection(SOCKET clisocket) : clisocket(clisocket), closing(0)
{
	sender = new TCPSender(this);
	receiver = new TCPReceiver(this);
	for (int i = 0; i < RDSERVICE_MAX; i++) {
		service[i] = RDServiceFactory::Instance()->CreateRDService(i, receiver->GetServiceRecvQueue(i), sender->GetServiceSendQueue(i), this);
		assert(service[i]);
	}
}

TCPConnection::~TCPConnection()
{
	for (int i = 0; i < RDSERVICE_MAX; i++) {
		service[i]->WaitThread();
	}
	for (int i = 0; i < RDSERVICE_MAX; i++) {
		delete service[i];
	}
	receiver->WaitThread();
	sender->WaitThread();
	delete receiver;
	delete sender;
	if (clisocket != INVALID_SOCKET) closesocket(clisocket);
	plog("connection closed.\n");
}

void TCPConnection::DoModal()
{
	if (!receiver->DoAuth()) {
		return;
	}

	for (int i = 0; i < RDSERVICE_MAX; i++) {
		service[i]->StartThread();
	}
	sender->StartThread();
	receiver->StartThread();
	
	for (int i = 0; i < RDSERVICE_MAX; i++) {
		service[i]->WaitThread();
	}
	sender->WaitThread();
	receiver->WaitThread();
}

void TCPConnection::Close()
{
	closing = 1;
	for (int i = 0; i < RDSERVICE_MAX; i++) {
		service[i]->NotifyClose();
	}
	sender->NotifyClose();
	receiver->NotifyClose();
	
	closesocket(clisocket);
	clisocket = INVALID_SOCKET;
}


int TCPConnection::SendAll(const char *data, size_t len)
{
	while (len > 0) {
		if (closing) return -1;
		int r = send(clisocket, data, len, 0);
		if (r == SOCKET_ERROR) {
			return -1;
		}
		data += r;
		len -= r;
	}
	return 0;
}

int TCPConnection::RecvAll(char *data, size_t len)
{
	while (len > 0) {
		if (closing) return -1;
		int r = recv(clisocket, data, len, 0);
		if (r == SOCKET_ERROR) {
			return -1;
		}
		data += r;
		len -= r;
	}
	return 0;
}
