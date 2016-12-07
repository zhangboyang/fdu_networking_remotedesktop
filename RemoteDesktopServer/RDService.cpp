#include "stdafx.h"
#include "common.h"

RDService::RDService(ProducerConsumerQueue<MsgPacket*> *recvqueue, ProducerConsumerQueue<MsgPacket*> *sendqueue, TCPConnection *conn) : recvqueue(recvqueue), sendqueue(sendqueue), conn(conn)
{
}
RDService::~RDService()
{
}
void RDService::RequestExit()
{
	conn->Close();
}