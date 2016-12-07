#include "stdafx.h"
#include "common.h"

RDServiceFactory *RDServiceFactory::Instance()
{
	static RDServiceFactory inst;
	return &inst;
}

RDService *RDServiceFactory::CreateRDService(int type, ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue, TCPConnection *conn)
{
	switch (type) {
		case RDSERVICE_SCREENSENDER: return new RDScreenSender(recvqueue, sendqueue, conn);
		case RDSERVICE_CONTROLRECEIVER: return new RDControlReceiver(recvqueue, sendqueue, conn);
		default: return NULL;
	}
}
RDServiceFactory::RDServiceFactory()
{
}
RDServiceFactory::~RDServiceFactory()
{
}