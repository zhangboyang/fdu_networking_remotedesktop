#pragma once

class TCPConnection;
class MsgPacket;

class RDService : public ThreadHelper {
	TCPConnection *conn;
public:
	virtual void RequestExit();
	ProducerConsumerQueue<MsgPacket *> *recvqueue, *sendqueue;
	RDService(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue, TCPConnection *conn);
	virtual ~RDService();
	virtual void NotifyClose() = 0;
};
