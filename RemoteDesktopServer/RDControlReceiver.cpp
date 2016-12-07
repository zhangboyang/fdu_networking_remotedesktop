#include "stdafx.h"
#include "common.h"

RDControlReceiver::RDControlReceiver(ProducerConsumerQueue<MsgPacket *> *recvqueue, ProducerConsumerQueue<MsgPacket *> *sendqueue) :
	RDService(recvqueue, sendqueue)
{

}

RDControlReceiver::~RDControlReceiver()
{

}

void RDControlReceiver::ThreadProc()
{
	while (1) {
		MsgPacket *packet;
		if (recvqueue->Get(&packet) < 0) break;
		packet->Dump();
		delete packet;
	}
}

void RDControlReceiver::NotifyClose()
{
}