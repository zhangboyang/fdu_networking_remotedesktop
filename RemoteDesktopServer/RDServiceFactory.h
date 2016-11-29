#pragma once

class RDServiceFactory {
public:
	static RDServiceFactory *Instance();
	RDService *CreateService(int type, ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue);
private:
	RDServiceFactory();
	~RDServiceFactory();
	RDServiceFactory(RDServiceFactory const &);
	RDServiceFactory& operator= (RDServiceFactory const &);
};