#include "stdafx.h"
#include "common.h"

RDService::RDService(ProducerConsumerQueue<MsgPacket*> *recvqueue, ProducerConsumerQueue<MsgPacket*> *sendqueue) : recvqueue(recvqueue), sendqueue(sendqueue)
{
}
RDService::~RDService()
{
}