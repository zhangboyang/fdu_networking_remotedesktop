#pragma once

class RDControlReceiver : public RDService {
public:
	RDControlReceiver(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue);
	virtual void ThreadProc();
	virtual void NotifyClose();
	virtual ~RDControlReceiver();
};