#include "stdafx.h"
#include "common.h"

RDServiceFactory *RDServiceFactory::Instance()
{
	static RDServiceFactory inst;
	return &inst;
}

RDService *RDServiceFactory::CreateService(int type, ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue)
{
	switch (type) {
		case RDSERVICE_SCREENSENDER: return new RDScreenSender(recvqueue, sendqueue);
		case RDSERVICE_CONTROLRECEIVER: return new RDControlReceiver(recvqueue, sendqueue);
		default: return NULL;
	}
}
RDServiceFactory::RDServiceFactory()
{
}
RDServiceFactory::~RDServiceFactory()
{
}