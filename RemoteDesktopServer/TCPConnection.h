#pragma once

class TCPSender;
class TCPReceiver;

class TCPConnection {
	friend class TCPSender;
	friend class TCPReceiver;
private:
	SOCKET clisocket;
	TCPSender *sender;
	TCPReceiver *receiver;
	int closing;
	RDService *service[RDSERVICE_MAX];
	int SendAll(const char *data, size_t len);
	int RecvAll(char *data, size_t len);
	int WaitEvent(HANDLE e);
	void PumpEvent();
public:
	TCPConnection(SOCKET clisocket);
	~TCPConnection();
	void DoModal();
	void Close();
};