#include "stdafx.h"
#include "common.h"

TCPReceiver::TCPReceiver(TCPConnection *conn) : conn(conn), recvqueue(RDSERVICE_MAX, MAX_QUEUED_PACKET)
{
}

void TCPReceiver::ThreadProc()
{
	plog("receiver thread created.\n");
	MsgPacket *pkt = NULL;

	while (1) {
		// FIXME: raw_header
		// receive data length
		MsgPacket::raw_header pkthdr;

		int ret = conn->RecvAll((char *)&pkthdr, sizeof(MsgPacket::raw_header));
		if (ret < 0) break;

		// receive data
		pkt = new MsgPacket;
		if (pkt->LoadHeader(&pkthdr) < 0) {
			plog("invalid packet header.\n");
			break;
		}
		char *buf = pkt->LockBuffer();
		ret = conn->RecvAll(buf, pkt->GetBufferSize());
		int type = pkt->GetType();
		pkt->UnlockBuffer();
		if (ret < 0) break;

		// dispatch packet
		if (type < 0 || type >= RDSERVICE_MAX) {
			plog("invalid packet type %d.\n", type);
			break;
		}
		recvqueue.GetIndividualQueue(type)->Append(pkt);

		// set pkt to NULL, since we dispatched the packet
		pkt = NULL;
	}

	if (pkt) delete pkt;
	conn->Close();
	plog("receiver thread exited.\n");
}

void TCPReceiver::NotifyClose()
{
	recvqueue.NotifyClose();
}

ProducerConsumerQueue<MsgPacket *> *TCPReceiver::GetServiceRecvQueue(int id)
{
	return recvqueue.GetIndividualQueue(id);
}