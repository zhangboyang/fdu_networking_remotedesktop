#pragma once

class TCPConnection;
class RDServiceFactory {
public:
	static RDServiceFactory *Instance();
	RDService *CreateRDService(int type, ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue, TCPConnection *conn);
private:
	RDServiceFactory();
	~RDServiceFactory();
	RDServiceFactory(RDServiceFactory const &);
	RDServiceFactory& operator= (RDServiceFactory const &);
};