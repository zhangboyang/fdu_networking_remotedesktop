#pragma once

class MsgPacket;

class RDService : public ThreadHelper {
public:
	ProducerConsumerQueue<MsgPacket *> *recvqueue, *sendqueue;
	RDService(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue);
	virtual ~RDService();
	virtual void NotifyClose() = 0;
};
